#include <complex.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>
#include "escapetime.h"
#include <math.h>
#include "../utils.h"
#include "../colormap.h"


void tricorn_V3(float complex start, size_t width, ssize_t height, float res, unsigned n, bool color, unsigned char* img) {

    int bytesPerPixel = color ? 3 : 1;

    // Größe einer Reihe wird durch Padding ein Vielfaches von 4
    size_t row_size = ((width * bytesPerPixel+3)/4)*4;

    // Die Richtung der y-Achse hängt vom Vorzeichen von 'height' ab:
    // height > 0: Start unten links, y wird nach oben größer
    // height < 0: Start oben links, y wird nach unten größer
    float factor = height < 0 ? -1.0f : 1.0f;
    size_t top = abs_size(height);

    // SIMD verarbeitet immer 4 Pixel gleichzeitig.
    // endIndex ist die größte Spalte, welche durch 4 teilbar ist.
    // Verbleibende 1 bis 3 Pixel werden am Ende aufgefüllt.
    size_t endIndex = width - (width % 4);

    /* SYMMETRIEOPTIMIERUNG:
     *
     * Die Symmetrie kann nur verwendet werden, wenn die gespiegelte Position exakt einer gültigen Bildposition entspricht.
     * Dadurch entstehen keine falschen Zuordnungen, aber die Symmetrie kann nicht für alle Auflösungen genutzt werden.
     *
     * Das Fraktal ist bezüglich der x(reellen)-Achse symmetrisch. Somit besitzen (x,y) und (x,-y) dieselbe Escape-Time.
     * Zunächst wird die Bildzeile bestimmt, in der der Imaginärteil 0 ist.
     * Für jede Zeile y wird dann ihre gespiegelte Zeile berechnet.
     *
     * Liegt diese oberhalb der aktuellen Zeile, wird später das Ergebnis gleichzeitig in beide
     * Zeilen geschrieben.
     *
     * Liegt sie unterhalb, wurde sie bereits berechnet/beschrieben und die Zeile kann übersprungen
     * werden.
     */

    double center = -(cimag(start) / (double) (factor * res));
    for (size_t y = 0; y < top; y++) {
        // *row zeigt nun auf den Anfang von Zeile y (0-indexed)
        unsigned char *bottomRow = img + y * row_size;

        // Berechnung der gespiegelten Zeile
        ssize_t sym_y = lround(2.0 * center - (double) y);

        // Imaginärteile der beiden Zeilen
        float imag_y = cimagf(start) + factor * (float) y * res;
        float imag_sym = cimagf(start) + factor * (float) sym_y * res;

        // Symmetrie wird nur verwendet, wenn die Spiegelung exakt ist
        // und die gespiegelte Zeile innerhalb des Bildes liegt.
        bool useSym = sym_y >= 0 && sym_y < (ssize_t) top && imag_y == -imag_sym ;
        unsigned char *topRow = NULL;

        // Koordinate schon berechnet und eingetragen.
        if(useSym && sym_y < (ssize_t) y) {
            continue;
        }

        // Ergebnis zusätzlich in gespiegelte Zeile schreiben.
        if (useSym && sym_y != (ssize_t) y) {
            topRow = img + sym_y * row_size;
        }

        // Verarbeitung der Pixel in 4er-Blöcken mittels SIMD
        for (size_t x = 0; x < endIndex; x += 4) {

            unsigned iterations[4];
            const float complex c1 = start + (float) x * res + factor * (float) y * res * I;
            const float complex c2 = start + ((float) x + 1) * res + factor * (float) y * res * I;
            const float complex c3 = start + ((float) x + 2) * res + factor * (float) y * res * I;
            const float complex c4 = start + ((float) x + 3) * res + factor * (float) y * res * I;

            // Escape Time für alle 4 Punkte gleichzeitig berechnen
            _mm_storeu_si128((__m128i*) iterations, symEscapeTime(n, c1, c2, c3, c4));

            // Ergebnisse ins Bild schreiben
            for (int i = 0; i < 4; i++){
                size_t x_offset = (x + i) * bytesPerPixel;
                unsigned char *bottomOffset = bottomRow + x_offset;
                RGB_color rgb = getColor(n, iterations[i], color);
                // BMP speichert im BGR-Format
                if (color) {
                    bottomOffset[0] =  rgb.b;
                    bottomOffset[1] =  rgb.g;
                    bottomOffset[2] =  rgb.r;

                    // Falls vorhanden, wird auch das gespiegelte Pixel gesetzt.
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
        // Den Rest von width auch noch fillen (bis zu 3 elemente).
        // Es wird weiterhin SIMD genutzt, wobei nur die tatsächlich in der Bitmap
        // vorkommenden Pixel übernommen werden
        if(endIndex < width) {
            unsigned rest_iterations[4];
            const float complex c1 = start + (float) endIndex * res + factor * (float) y * res * I;
            const float complex c2 = start + ((float) endIndex + 1) * res + factor * (float) y * res * I;
            const float complex c3 = start + ((float) endIndex + 2) * res + factor * (float) y * res * I;
            const float complex c4 = start + ((float) endIndex + 3) * res + factor * (float) y * res * I;
            _mm_storeu_si128((__m128i*) rest_iterations, symEscapeTime(n, c1, c2, c3, c4));

            size_t range = width - endIndex;
            for (size_t i = 0; i < range; i++){
                size_t x_offset = (endIndex + i) * bytesPerPixel;
                unsigned char *bottomOffset = bottomRow + x_offset;
                RGB_color rgb = getColor(n, rest_iterations[i], color);
                // BMP speichert im BGR-Format
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
        //restliche Padding Bits einer Zeile auffüllen
        for (size_t i= width*bytesPerPixel; i<row_size; i++) {
            bottomRow[i]=0;
            if (topRow != NULL) {
                topRow[i]=0;
            }
        }
    }
}
