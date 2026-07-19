#include <complex.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <immintrin.h>
#include <pthread.h>

#include "../LatencyThroughput/escapetime.h"
#include "../LatencyThroughput/tricorn_V4.h"
#include "corecalculator.h"
#include "../colormap.h"


typedef struct{
    float complex start;
    size_t width;
    ssize_t height;
    size_t from_y;
    size_t to_y;
    size_t step_size;
    float res;
    unsigned n;
    bool color;
    unsigned char* img;
} thread_data;

static void* tricornThreading(void* arg) {
    thread_data* values  = arg;

    float complex start = values->start;
    size_t width = values->width;
    ssize_t height = values->height;
    size_t from_y = values->from_y;
    size_t to_y = values->to_y;
    size_t step_size = values->step_size;
    float res = values->res;
    unsigned n = values->n;
    bool color = values->color;
    unsigned char* img = values->img;
    int bytes_per_pixel = color ? 3 : 1 ;
    // Größe einer Reihe wird durch Padding ein Vielfaches von 4
    size_t row_size = ((width * bytes_per_pixel+3)/4)*4;

    // Die Richtung der y-Achse hängt vom Vorzeichen von 'height' ab:
    // height > 0: Start unten links, y wird nach oben größer
    // height < 0: Start oben links, y wird nach unten größer
    float factor = height < 0 ? -1.0f : 1.0f;

    // SIMD verarbeitet hier immer 4*2 Pixel gleichzeitig.
    // endIndex ist die größte Spalte, welche durch 8 teilbar ist.
    // Verbleibende 1 bis 7 Pixel werden am Ende aufgefüllt.
    size_t end_index = width - (width % 8);

    for (size_t y = from_y; y < to_y; y += step_size) {

        // *row zeigt nun auf den Anfang von Zeile y (0-indexed)
        unsigned char *bottom_row = img + y * row_size;

        // Verarbeitung der Pixel in 8er-Blöcken mittels SIMD
        for (size_t x = 0; x < end_index; x += 8) {
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

            // Escape Time für alle 8 Punkte gleichzeitig berechnen
            twoIntegerLanes both_iterations = escapeTimeLatencyThroughputTwo(n, a1, a2, a3, a4, b1, b2, b3, b4);
            _mm_storeu_si128((__m128i*)iterations,       both_iterations.a);
            _mm_storeu_si128((__m128i*)(iterations + 4), both_iterations.b);

            // Ergebnisse ins Bild schreiben
            for (int i = 0; i < 8; i++){
                size_t x_offset = (x + i) * bytes_per_pixel;
                unsigned char *bottom_offset = bottom_row + x_offset;
                RGB_color rgb = getColor(n, iterations[i], color);

                // BMP speichert im BGR-Format
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


        // Den Rest von width auch noch fillen (bis zu 7 elemente).
        // Es wird weiterhin SIMD genutzt, wobei nur die tatsächlich in der Bitmap
        // vorkommenden Pixel übernommen werden
        if(end_index < width) {
            unsigned rest_iterations[8];
            float complex imag_part = factor * (float) y * res * I;
            const float complex a1 = start + (float) end_index * res + imag_part;
            const float complex a2 = start + ((float) end_index + 1) * res + imag_part;
            const float complex a3 = start + ((float) end_index + 2) * res + imag_part;
            const float complex a4 = start + ((float) end_index + 3) * res + imag_part;
            const float complex b1 = start + ((float) end_index + 4) * res + imag_part;
            const float complex b2 = start + ((float) end_index + 5) * res + imag_part;
            const float complex b3 = start + ((float) end_index + 6) * res + imag_part;
            const float complex b4 = start + ((float) end_index + 7) * res + imag_part;

            twoIntegerLanes both_iterations = escapeTimeLatencyThroughputTwo(n, a1, a2, a3, a4, b1, b2, b3, b4);
            _mm_storeu_si128((__m128i*)rest_iterations,       both_iterations.a);
            _mm_storeu_si128((__m128i*)(rest_iterations + 4), both_iterations.b);

            // Nur die Werte eintragen, die tatsächlich noch benötigt werden, Overflow wird ignoiert.
            size_t range = width - end_index;
            for (size_t i = 0; i < range; i++){
                size_t x_offset = (end_index + i) * bytes_per_pixel;
                unsigned char *bottom_offset = bottom_row + x_offset;
                RGB_color rgb = getColor(n, rest_iterations[i], color);

                // BMP speichert im BGR-Format
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

        // restliche Padding Bits einer Zeile auffüllen
        for (size_t i= width*bytes_per_pixel; i<row_size; i++) {
            bottom_row[i] = 0;
        }
    }
    return NULL;
}

void tricorn(float complex start, size_t width, ssize_t height, float res, unsigned n, bool color,
    unsigned char* img) {
    // Bestimmt die absolute Bildhöhe, da das Vorzeichen von 'height'
    // lediglich die Ausrichtung der y-Achse festlegt.
    size_t max_y = height > 0 ? height : -height;

    // Multithreading wird nur bei ausreichend großen Bildern verwendet.
    // Der Grenzwert von 200.000 wurde experimentall bestimmt.
    // Ab dieser Größe überwiegt der Geschwindigkeitsgewinn den Thread-Overhead.
    if(width * max_y > 200000){

        // Ermittelt die Anzahl der verfügbaren Prozessorkerne
        int thread_amount = getCoreAmount();

        // Speichert die Thread-Handles und die zugehörigen Argumente.
        pthread_t threads[thread_amount];
        thread_data data[thread_amount];

        // Erstellt für jeden verfügbaren Prozessorkern einen Thread.
        for (int i = 0; i < thread_amount; i++) {
            pthread_t thread;

            // Jeder Thread bearbeitet jede thread_amount-te Bildzeile.
            // i gibt die erste zu bestimmende Zeile an,
            // max_y die obere Grenze und thread_amount die Schrittweise.
            data[i] = (thread_data) {start, width, height, i, max_y, thread_amount, res, n, color, img};;
            pthread_create(&thread, NULL, tricornThreading, &data[i]);
            threads[i] = thread;
        }

        // Wartet, bis alle Threads fertig sind.
        for (int i = 0; i < thread_amount; i++) {
            pthread_join(threads[i], NULL);
        }
    } else {

        //Bei kleinen Bildern wird einfach die vorherige Implementation,
        // also die 'Latency' Implementation (V4) genutzt.
        tricorn_V4(start, width, height, res, n, color, img);
    }
}
