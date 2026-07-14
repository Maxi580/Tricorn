#include <complex.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <immintrin.h>
#include "escapetime.h"
#include <math.h>
#include "../utils.h"

void tricorn_V5(float complex start, size_t width, ssize_t height, float res, unsigned n, bool color, unsigned char* img) {
    int bytesPerPixel = color ? 3 : 1;
    size_t row_size = ((width * bytesPerPixel+3)/4)*4; // Größe einer Reihe würde auf 4 Byte aufgefüllt(Padding)

    /* Wenn heigt kleiner 0 ist, startet man unten links und geht nach oben also addiert man +y und sonst
     * startet man oben links un zieht y ab, das vorzeichen wird über factor kontrolliert*/
    float factor = height < 0 ? -1.0f : 1.0f;
    ssize_t top = abs_size(height);

    /* Width muss nicht durch die Anzahl der gleichzeitig berechnet Elemente teilbar sein. Deswegen machen wir
       x koordinaten in parallel so lange das geht und dann am Ende den Rest*/
    size_t endIndex = width - (width % 8);

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
        if(useSym && sym_y < y) {
            continue; // Koordinate schon berechnet und eingetragen
        }

        if (useSym && sym_y != y) {
            topRow = img + sym_y * row_size;
        }

        for (size_t x = 0; x < endIndex; x += 8) {
            unsigned iterations[8];
            const float complex imag_part = factor * (float) y * res * I;
            const float complex a1 = start + (float) x * res + imag_part;
            const float complex a2 = start + ((float) x + 1) * res + imag_part;
            const float complex a3 = start + ((float) x + 2) * res + imag_part;
            const float complex a4 = start + ((float) x + 3) * res + imag_part;
            const float complex b1 = start + ((float) x + 4) * res + imag_part;
            const float complex b2 = start + ((float) x + 5) * res + imag_part;
            const float complex b3 = start + ((float) x + 6) * res + imag_part;
            const float complex b4 = start + ((float) x + 7) * res + imag_part;


            twoIntegerLanes bothIterations = escapeTimeLatencyThroughputTwo(n, a1, a2, a3, a4, b1, b2, b3, b4);
            _mm_storeu_si128((__m128i*)iterations,       bothIterations.a);
            _mm_storeu_si128((__m128i*)(iterations + 4), bothIterations.b);

            for (int i = 0; i < 8; i++){
                size_t x_offset = (x + i) * bytesPerPixel;
                unsigned char *bottomOffset = bottomRow + x_offset;
                RGB_color rgb = getColor(n, iterations[i], color);

                // rgb is reversed on bitmap
                if (color) {
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
                else {
                    bottomOffset[0] =  rgb.b;
                    if(topRow != NULL){
                        unsigned char *topOffset = topRow + x_offset;
                        topOffset[0] =  rgb.b;
                    }
                }
            }
        }
        // Den Rest von width auch noch fillen (bis zu (ParallelElemente - 1) elemente)
        // Es werden 4 werte in parallel berechnet, aber nur die benötigten eingetragen
        if(endIndex < width) {
            unsigned rest_iterations[8];
            float complex imag_part = factor * (float) y * res * I;
            const float complex a1 = start + (float) endIndex * res + imag_part;
            const float complex a2 = start + ((float) endIndex + 1) * res + imag_part;
            const float complex a3 = start + ((float) endIndex + 2) * res + imag_part;
            const float complex a4 = start + ((float) endIndex + 3) * res + imag_part;
            const float complex b1 = start + ((float) endIndex + 4) * res + imag_part;
            const float complex b2 = start + ((float) endIndex + 5) * res + imag_part;
            const float complex b3 = start + ((float) endIndex + 6) * res + imag_part;
            const float complex b4 = start + ((float) endIndex + 7) * res + imag_part;

            twoIntegerLanes bothIterations = escapeTimeLatencyThroughputTwo(n, a1, a2, a3, a4, b1, b2, b3, b4);
            _mm_storeu_si128((__m128i*)rest_iterations,       bothIterations.a);
            _mm_storeu_si128((__m128i*)(rest_iterations + 4), bothIterations.b);

            // Nur die Werte eintragen, die tatsächlich noch benötigt werden, Overflow wird ignoiert.
            size_t range = width - endIndex;
            for (size_t i = 0; i < range; i++){
                size_t x_offset = (endIndex + i) * bytesPerPixel;
                unsigned char *bottomOffset = bottomRow + x_offset;
                RGB_color rgb = getColor(n, rest_iterations[i], color);
                // rgb is reversed on bitmap
                if (color) {
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
                else {
                    bottomOffset[0] =  rgb.b;
                    if(topRow != NULL){
                        unsigned char *topOffset = topRow + x_offset;
                        topOffset[0] =  rgb.b;
                    }
                }
            }
        }

        for (size_t i= width*bytesPerPixel; i<row_size; i++) {
            bottomRow[i]=0; // restliche Padding Bits einer Zeile auffüllen
            if (topRow != NULL) {
                topRow[i]=0;
            }
        }
    }
}
