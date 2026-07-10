#include <complex.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>
#include "escapetime.h"

#define rgbSize 3


const unsigned colorPallete[4][rgbSize] = {
    // We call modulo with the size of this pallete, quite a few times, it would be nicer if this is a multiple of 2 ig
    // Blue, Green, Red
    {128, 0, 0},
    {255, 0, 0},
    {255, 100, 0},
    {255, 255, 0},
};
const unsigned colorPalleteSize = sizeof(colorPallete) / sizeof(colorPallete[0]);


__attribute__((hot))
inline void colorizeBitmap(unsigned limit, unsigned iterations, unsigned char* row, bool color)  {
    /* Colors one Pixel in the Bitmap */
    if (limit == iterations) {
        // Black (Did Not converge)
        row[0] = row[1] = row[2] = 0;
    } else {
        // Branch Prediction should be correct almost always, so i assume that this doesnt cost anything really
        if (!color) {
            // Grey
            unsigned colorPixel = 255 - 4 * ( iterations % 64 );
            row[0] = row[1] = row[2] = colorPixel;
        } else {
            // Colorized
            unsigned colorIndex = iterations % colorPalleteSize;
            row[0]= colorPallete[colorIndex][0];
            row[1]= colorPallete[colorIndex][1];
            row[2]= colorPallete[colorIndex][2];
        }
    }
}

void tricornSimd(float complex start, size_t width, ssize_t height, float res, unsigned n, bool color, unsigned char* img) {
    size_t row_size = ((width * 3+3)/4)*4; // Größe einer Reihe würde auf 4 Byte aufgefüllt(Padding)

    /* Wenn heigt kleiner 0 ist, startet man unten links und geht nach oben also addiert man +y und sonst
     * startet man oben links un zieht y ab, das vorzeichen wird über factor kontrolliert*/
    float factor = height < 0 ? -1.0f : 1.0f;
    ssize_t top = height < 0 ? -height : height;

    /* Width muss nicht durch 4 teilbar sein. Deswegen machen wir 4 koordinaten in parallel so lange das geht und
     * dann am Ende den Rest (also 0 - 3 Elemente) sequenziell*/
    size_t endIndex = width - (width % 4);
    for (ssize_t y = 0; y < top; y++) {
        unsigned char *row = img + y*row_size; // *row zeigt nun auf den Anfang von Zeile y (0-indexed)
        for (size_t x = 0; x < endIndex; x += 4) {
            unsigned iterations[4];
            const float complex c1 = start + (float) x * res + factor * (float) y * res * I;
            const float complex c2 = start + ((float) x + 1) * res + factor * (float) y * res * I;
            const float complex c3 = start + ((float) x + 2) * res + factor * (float) y * res * I;
            const float complex c4 = start + ((float) x + 3) * res + factor * (float) y * res * I;
            _mm_storeu_si128((__m128i*) iterations, simdEscapeTime(n, c1, c2, c3, c4));

            for (int i = 0; i < 4; i++){
                colorizeBitmap(n, iterations[i], row + ((x + i) * 3), color);
            }
        }
        // Den Rest von width auch noch fillen (bis zu 3 elemente)
        // Es werden 4 werte in parallel berechnet, aber nur die benötigten eingetragen
        if(endIndex < width) {
            unsigned restIterations[4];
            const float complex c1 = start + (float) endIndex * res + factor * (float) y * res * I;
            const float complex c2 = start + ((float) endIndex + 1) * res + factor * (float) y * res * I;
            const float complex c3 = start + ((float) endIndex + 2) * res + factor * (float) y * res * I;
            const float complex c4 = start + ((float) endIndex + 3) * res + factor * (float) y * res * I;
            _mm_storeu_si128((__m128i*) restIterations, simdEscapeTime(n, c1, c2, c3, c4));

            for (size_t i = 0; i < (width - endIndex); i++) {
                colorizeBitmap(n, restIterations[i], row + ((endIndex + i) * 3), color);
            }
        }

        for (size_t i= width*3; i<row_size; i++) {
            row[i]=0; //restliche Padding Bits einer Zeile auffüllen
        }
    }
}
