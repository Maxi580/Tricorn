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
    int bytes_per_pixel = color ? 3 : 1;

    // Größe einer Reihe wird auf 4 Byte aufgefüllt(Padding)
    size_t row_size = ((width * bytes_per_pixel+3)/4)*4;

    // Die Richtung der y-Achse hängt vom Vorzeichen von 'height' ab:
    // height > 0: Start unten links, y wird nach oben größer
    // height < 0: Start oben links, y wird nach unten größer
    float factor = height < 0 ? -1.0f : 1.0f;
    ssize_t top = abs_size(height);

    // SIMD verarbeitet immer 4 Pixel gleichzeitig.
    // endIndex ist die größte Spalte, welche durch 4 teilbar ist
    // Restpixel in einer Reihe müssen somit seperat (ohne SIMD) berechnet werden
    size_t end_index = width - (width % 4);


    for (ssize_t y = 0; y < top; y++) {
        // *row zeigt nun auf den Anfang von Zeile y (0-indexed)
        unsigned char *row = img + y*row_size;

        // Verarbeitung der Pixel in 4er-Blöcken mittels SIMD
        for (size_t x = 0; x < end_index; x += 4) {
            unsigned iterations[4];
            const float complex c1 = start + (float) x * res + factor * (float) y * res * I;
            const float complex c2 = start + ((float) x + 1) * res + factor * (float) y * res * I;
            const float complex c3 = start + ((float) x + 2) * res + factor * (float) y * res * I;
            const float complex c4 = start + ((float) x + 3) * res + factor * (float) y * res * I;

            // Escape Time für alle 4 Punkte gleichzeitig berechnen
            _mm_storeu_si128((__m128i*) iterations, simdEscapeTime(n, c1, c2, c3, c4));

            // Ergebnisse ins Bild schreiben
            for (int i = 0; i < 4; i++){
                size_t x_offset = (x + i) * bytes_per_pixel;
                unsigned char *bottom_offset = row + x_offset;
                RGB_color rgb = getColor(n, iterations[i], color);
                // BMP speichert im BGR-Format
                if (!color) {
                    bottom_offset[0] =  rgb.b;
                }
                else {
                    bottom_offset[0] =  rgb.b;
                    bottom_offset[1] =  rgb.g;
                    bottom_offset[2] =  rgb.r;
                }

            }
        }
        // Den Rest von width auch noch fillen (bis zu 3 elemente).
        // Es wird weiterhin SIMD genutzt, wobei nur die tatsächlich in der Bitmap
        // vorkommenden Pixel übernommen werden
        if(end_index < width) {
            unsigned rest_iterations[4];
            const float complex c1 = start + (float) end_index * res + factor * (float) y * res * I;
            const float complex c2 = start + ((float) end_index + 1) * res + factor * (float) y * res * I;
            const float complex c3 = start + ((float) end_index + 2) * res + factor * (float) y * res * I;
            const float complex c4 = start + ((float) end_index + 3) * res + factor * (float) y * res * I;
            _mm_storeu_si128((__m128i*) rest_iterations, simdEscapeTime(n, c1, c2, c3, c4));

            for (size_t i = 0; i < (width - end_index); i++){
                size_t x_offset = (end_index + i) * bytes_per_pixel;
                unsigned char *bottom_offset = row + x_offset;
                RGB_color rgb = getColor(n, rest_iterations[i], color);
                // BMP speichert im BGR-Format
                if (!color) {
                    bottom_offset[0] =  rgb.b;
                }
                else {
                    bottom_offset[0] =  rgb.b;
                    bottom_offset[1] =  rgb.g;
                    bottom_offset[2] =  rgb.r;
                }

            }
        }
        //restliche Padding Bits einer Zeile auffüllen
        for (size_t i= width*bytes_per_pixel; i<row_size; i++) {
            row[i]=0;
        }
    }
}
