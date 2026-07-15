#include <complex.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>
#include "escapetime.h"
#include "../utils.h"
#include "../colormap.h"


void tricorn_V2(float complex start, size_t width, ssize_t height, float res, unsigned n, bool color, unsigned char* img) {
    int bytesPerPixel = color ? 3 : 1;

    // Größe einer Reihe wird auf 4 Byte aufgefüllt(Padding)
    size_t row_size = ((width * bytesPerPixel+3)/4)*4;

    // Die Richtung der y-Achse hängt vom Vorzeichen von 'height' ab:
    // height > 0: Start unten links, y wird nach oben größer
    // height < 0: Start oben links, y wird nach unten größer
    float factor = height < 0 ? -1.0f : 1.0f;
    ssize_t top = abs_size(height);

    // SIMD verarbeitet immer 4 Pixel gleichzeitig.
    // endIndex ist die größte Spalte, welche durch 4 teilbar ist
    // Restpixel in einer Reihe müssen somit seperat (ohne SIMD) berechnet werden
    size_t endIndex = width - (width % 4);


    for (ssize_t y = 0; y < top; y++) {

        // *row zeigt nun auf den Anfang von Zeile y (0-indexed)
        unsigned char *row = img + y*row_size;

        // Verarbeitung der Pixel in 4er-Blöcken mittels SIMD
        for (size_t x = 0; x < endIndex; x += 4) {
            unsigned iterations[4];
            const float complex c1 = start + (float) x * res + factor * (float) y * res * I;
            const float complex c2 = start + ((float) x + 1) * res + factor * (float) y * res * I;
            const float complex c3 = start + ((float) x + 2) * res + factor * (float) y * res * I;
            const float complex c4 = start + ((float) x + 3) * res + factor * (float) y * res * I;

            // Escape Time für alle 4 Punkte gleichzeitig berechnen
            _mm_storeu_si128((__m128i*) iterations, simdEscapeTime(n, c1, c2, c3, c4));

            // Ergebnisse ins Bild schreiben
            for (int i = 0; i < 4; i++){
                size_t x_offset = (x + i) * bytesPerPixel;
                unsigned char *bottomOffset = row + x_offset;
                RGB_color rgb = getColor(n, iterations[i], color);
                // BMP speichert im BGR-Format
                if (!color) {
                    bottomOffset[0] =  rgb.b;
                }
                else {
                    bottomOffset[0] =  rgb.b;
                    bottomOffset[1] =  rgb.g;
                    bottomOffset[2] =  rgb.r;
                }

            }
        }
        // Den Rest von width auch noch fillen (bis zu 3 elemente).
        // Es wird weiterhin SIMD genutzt, wobei nur die tatsächlich in der Bitmap
        // vorkommenden Pixel übernommen werden
        if(endIndex < width) {
            unsigned restIterations[4];
            const float complex c1 = start + (float) endIndex * res + factor * (float) y * res * I;
            const float complex c2 = start + ((float) endIndex + 1) * res + factor * (float) y * res * I;
            const float complex c3 = start + ((float) endIndex + 2) * res + factor * (float) y * res * I;
            const float complex c4 = start + ((float) endIndex + 3) * res + factor * (float) y * res * I;
            _mm_storeu_si128((__m128i*) restIterations, simdEscapeTime(n, c1, c2, c3, c4));

            for (size_t i = 0; i < (width - endIndex); i++){
                size_t x_offset = (endIndex + i) * bytesPerPixel;
                unsigned char *bottomOffset = row + x_offset;
                RGB_color rgb = getColor(n, restIterations[i], color);
                // BMP speichert im BGR-Format
                if (!color) {
                    bottomOffset[0] =  rgb.b;
                }
                else {
                    bottomOffset[0] =  rgb.b;
                    bottomOffset[1] =  rgb.g;
                    bottomOffset[2] =  rgb.r;
                }

            }
        }
        //restliche Padding Bits einer Zeile auffüllen
        for (size_t i= width*bytesPerPixel; i<row_size; i++) {
            row[i]=0;
        }
    }
}
