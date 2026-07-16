#include <complex.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <immintrin.h>
#include "escapetime.h"
#include <math.h>
#include "../utils.h"
#include "../colormap.h"


void tricorn_V3(float complex start, size_t width, ssize_t height, float res, unsigned n, bool color, unsigned char* img) {
    int bytes_per_pixel = color ? 3 : 1;
    size_t row_size = ((width * bytes_per_pixel+3)/4)*4; // Größe einer Reihe würde auf 4 Byte aufgefüllt(Padding)

    /* Wenn heigt kleiner 0 ist, startet man unten links und geht nach oben also addiert man +y und sonst
     * startet man oben links un zieht y ab, das vorzeichen wird über factor kontrolliert*/
    float factor = height < 0 ? -1.0f : 1.0f;
    size_t top = abs_size(height);

    /* Width muss nicht durch 4 teilbar sein. Deswegen machen wir 4 koordinaten in parallel so lange das geht und
     * dann am Ende den Rest (also 0 - 3 Elemente) sequenziell*/
    size_t end_index = width - (width % 4);

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
    for (size_t y = 0; y < top; y++) {
        unsigned char *bottom_row = img + y * row_size; // *row zeigt nun auf den Anfang von Zeile y (0-indexed)

        for (size_t x = 0; x < end_index; x += 4) {
            unsigned iterations[4];
            const float complex c1 = start + (float) x * res + factor * (float) y * res * I;
            const float complex c2 = start + ((float) x + 1) * res + factor * (float) y * res * I;
            const float complex c3 = start + ((float) x + 2) * res + factor * (float) y * res * I;
            const float complex c4 = start + ((float) x + 3) * res + factor * (float) y * res * I;
            _mm_storeu_si128((__m128i*) iterations, escapeTimeCircle(n, c1, c2, c3, c4));

            for (int i = 0; i < 4; i++){
                size_t x_offset = (x + i) * bytes_per_pixel;
                unsigned char *bottom_offset = bottom_row + x_offset;
                RGB_color rgb = getColor(n, iterations[i], color);

                // rgb is reversed on bitmap
                if (color) {
                    bottom_offset[0] =  rgb.b;
                    bottom_offset[1] =  rgb.g;
                    bottom_offset[2] =  rgb.r;
                }
                else {
                    bottom_offset[0] =  rgb.b;
                }
            }
        }
        // Den Rest von width auch noch fillen (bis zu 3 elemente)
        // Es werden 4 werte in parallel berechnet, aber nur die benötigten eingetragen
        if(end_index < width) {
            unsigned rest_iterations[4];
            const float complex c1 = start + (float) end_index * res + factor * (float) y * res * I;
            const float complex c2 = start + ((float) end_index + 1) * res + factor * (float) y * res * I;
            const float complex c3 = start + ((float) end_index + 2) * res + factor * (float) y * res * I;
            const float complex c4 = start + ((float) end_index + 3) * res + factor * (float) y * res * I;
            _mm_storeu_si128((__m128i*) rest_iterations, escapeTimeCircle(n, c1, c2, c3, c4));

            size_t range = width - end_index;
            for (size_t i = 0; i < range; i++){
                size_t x_offset = (end_index + i) * bytes_per_pixel;
                unsigned char *bottom_offset = bottom_row + x_offset;
                RGB_color rgb = getColor(n, rest_iterations[i], color);

                if (color) {
                    bottom_offset[0] =  rgb.b;
                    bottom_offset[1] =  rgb.g;
                    bottom_offset[2] =  rgb.r;
                }
                else {
                    bottom_offset[0] =  rgb.b;
                }
            }
        }

        for (size_t i= width*bytes_per_pixel; i<row_size; i++) {
            bottom_row[i]=0; //restliche Padding Bits einer Zeile auffüllen
        }
    }
}
