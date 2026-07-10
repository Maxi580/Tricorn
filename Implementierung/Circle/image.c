#include <complex.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>
#include "escapetime.h"
#include <math.h>
#include "escapetime.h"
#include "escapetime.c"

#define rgbSize 3

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} RGB_Color;


static const RGB_Color colorPallete[4] = {
    // We call modulo with the size of this pallete, quite a few times, it would be nicer if this is a multiple of 2 ig
    // Red, Green, Blue
    {0, 0, 128},
    {0, 0, 255},
    {0, 100, 255},
    {0, 255, 255},
};
static const unsigned colorPalleteSize = sizeof(colorPallete) / sizeof(colorPallete[0]);

static RGB_Color* createColorMap(unsigned limit, bool color) {
    RGB_Color* mapping = malloc((limit + 1) * sizeof(RGB_Color));
    if (mapping == NULL) {
        return NULL;
    }

    for (unsigned i = 0; i < limit; i++) {
        if (color) {
            unsigned colorIndex = i % colorPalleteSize;
            mapping[i] = colorPallete[colorIndex];
        } else {
            unsigned greyColor = 255 - 4 * (i % 64);
            mapping[i] = (RGB_Color) {greyColor, greyColor, greyColor};
        }
    }
    mapping[limit] = (RGB_Color) {0, 0, 0};
    return mapping;
}

void tricornCircle(float complex start, size_t width, ssize_t height, float res, unsigned n, bool color, unsigned char* img) {
    RGB_Color* colorMap = createColorMap(n, color);
    if (colorMap == NULL) {
        printf("Error allocating memory\n");
        free(colorMap);
        return;
    }

    size_t row_size = ((width * 3+3)/4)*4; // Größe einer Reihe würde auf 4 Byte aufgefüllt(Padding)

    /* Wenn heigt kleiner 0 ist, startet man unten links und geht nach oben also addiert man +y und sonst
     * startet man oben links un zieht y ab, das vorzeichen wird über factor kontrolliert*/
    float factor = height < 0 ? -1.0f : 1.0f;
    ssize_t top = height < 0 ? -height : height;

    /* Width muss nicht durch 4 teilbar sein. Deswegen machen wir 4 koordinaten in parallel so lange das geht und
     * dann am Ende den Rest (also 0 - 3 Elemente) sequenziell*/
    size_t endIndex = width - (width % 4);

    /* For Symmetre we first find the center which is the y where imag is 0, then knowing that,
       we for every y can calculate the corresponding mirrored y which is 2 * center - y (makes sense)
       Now we calculate the same stuff as before, however if we find a partner, that is smaller than the current y
       this must mean that it already was computed, as we go from 0 to top i guess. The only issue is now that
       if the center is out of bounds. When it is below the image with our formula this would mean that we already
        have calculated everything. So we only skip if the partner is larger than 0. If it is above the image, we dont
       skip anything, but we obviously cant save anyhting either. We must consider that we can only draw pixels in bound
       so we check both later.

        HOWEVER. If due to weird resolutions etc the center is not at a clean y but at like 2.4 or something
        i see no way of using that optimization.
     */
    double center = -(cimag(start) / (double) (factor * res));
    for (ssize_t y = 0; y < top; y++) {
        unsigned char *bottomRow = img + y * row_size; // *row zeigt nun auf den Anfang von Zeile y (0-indexed)

        // Berechnen vom Imaginärteil von y und des kandidaten, Symmetrie wird nur bei exakter korrektheit verwendet
        ssize_t sym_y = lround(2.0 * center - (double) y);
        float imag_y = cimagf(start) + factor * (float) y * res;
        float imag_sym = cimagf(start) + factor * (float) sym_y * res;
        // Durch Exaktheit keine False Positives aber auch weniger Hits
        bool useSym = sym_y >= 0 && sym_y < top && imag_y == -imag_sym ;
        unsigned char *topRow = NULL;
        if(useSym && sym_y < (ssize_t) y) {
            continue; // Koordinate schon berechnet und eingetragen
        }

        if (useSym && sym_y != (ssize_t) y) {
            topRow = img + sym_y * row_size;
        }

        for (size_t x = 0; x < endIndex; x += 4) {
            unsigned iterations[4];
            const float complex c1 = start + (float) x * res + factor * (float) y * res * I;
            const float complex c2 = start + ((float) x + 1) * res + factor * (float) y * res * I;
            const float complex c3 = start + ((float) x + 2) * res + factor * (float) y * res * I;
            const float complex c4 = start + ((float) x + 3) * res + factor * (float) y * res * I;
            _mm_storeu_si128((__m128i*) iterations, escapeTimeCircle(n, c1, c2, c3, c4));

            for (int i = 0; i < 4; i++){
                size_t x_offset = (x + i) * 3;
                unsigned char *bottomOffset = bottomRow + x_offset;
                RGB_Color rgb = colorMap[iterations[i]];
                // rgb is reversed on bitmap
                bottomOffset[0] =  rgb.b;
                bottomOffset[1] =  rgb.g;
                bottomOffset[2] =  rgb.r;
                if(topRow != NULL){
                    unsigned char *topOffset = topRow + x_offset;
                    topOffset[0] =  rgb.b;
                    topOffset[1] =  rgb.g;
                    topOffset[2] =  rgb.r;
                }
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
            _mm_storeu_si128((__m128i*) restIterations, escapeTimeCircle(n, c1, c2, c3, c4));

            size_t range = width - endIndex;
            for (size_t i = 0; i < range; i++){
                size_t x_offset = (endIndex + i) * 3;
                unsigned char *bottomOffset = bottomRow + x_offset;
                RGB_Color rgb = colorMap[restIterations[i]];
                // rgb is reversed on bitmap
                bottomOffset[0] =  rgb.b;
                bottomOffset[1] =  rgb.g;
                bottomOffset[2] =  rgb.r;
                if(topRow != NULL){
                    unsigned char *topOffset = topRow + x_offset;
                    topOffset[0] =  rgb.b;
                    topOffset[1] =  rgb.g;
                    topOffset[2] =  rgb.r;
                }
            }
        }

        for (size_t i= width*3; i<row_size; i++) {
            bottomRow[i]=0; //restliche Padding Bits einer Zeile auffüllen
            if (topRow != NULL) {
                topRow[i]=0;
            }
        }
    }
    free(colorMap);
}

/*
int main(void) {
    size_t width;
    width = 500; //breite eingeben
    ssize_t heightsigned = 700; //höhe eingeben
    size_t height;
    if (heightsigned < 0) {
        height = heightsigned * (-1);
    }
    else {
        height = heightsigned ;
    }

    size_t row_size = ((width * 3+3)/4)*4;
    size_t pixelArraySize = row_size * height;

    unsigned char *img = malloc(pixelArraySize);
    if (img == NULL) {return 1;} //Fehler

    genImage(width, heightsigned, img);
    writeBmp("test.bmp", width, heightsigned, img);
    free(img);
    return 0;
}
*/