#include <complex.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <immintrin.h>
#include <math.h>
#include <pthread.h>

#include "../LatencyThroughput/escapetime.h"
#include "../LatencyThroughput/tricorn_V4.h"
#include "../utils.h"
#include "corecalculator.h"
#include "../colormap.h"
#include "../utils.h"


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
    size_t row_size = ((width * bytes_per_pixel+3)/4)*4; // Größe einer Reihe würde auf 4 Byte aufgefüllt(Padding)

    /* Wenn heigt kleiner 0 ist, startet man unten links und geht nach oben also addiert man +y und sonst
     * startet man oben links un zieht y ab, das vorzeichen wird über factor kontrolliert*/
    float factor = height < 0 ? -1.0f : 1.0f;

    /* Width muss nicht durch die Anzahl der gleichzeitig berechnet Elemente teilbar sein. Deswegen machen wir
       x koordinaten in parallel so lange das geht und dann am Ende den Rest*/
    size_t end_index = width - (width % 8);

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
    for (size_t y = from_y; y < to_y; y += step_size) {
        unsigned char *bottom_row = img + y * row_size; // *row zeigt nun auf den Anfang von Zeile y (0-indexed)

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

            twoIntegerLanes both_iterations = escapeTimeLatencyThroughputTwo(n, a1, a2, a3, a4, b1, b2, b3, b4);
            _mm_storeu_si128((__m128i*)iterations,       both_iterations.a);
            _mm_storeu_si128((__m128i*)(iterations + 4), both_iterations.b);

            for (int i = 0; i < 8; i++){
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
        // Den Rest von width auch noch fillen (bis zu (ParallelElemente - 1) elemente)
        // Es werden 4 werte in parallel berechnet, aber nur die benötigten eingetragen
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

        for (size_t i= width*bytes_per_pixel; i<row_size; i++) {
            bottom_row[i] = 0; // restliche Padding Bits einer Zeile auffüllen
        }
    }
    return NULL;
}

void tricorn(float complex start, size_t width, ssize_t height, float res, unsigned n, bool color,
    unsigned char* img) {
    /* Multithreaded entlang der Y-Achse */
    size_t max_y = height > 0 ? height : -height;

    //  The 200k Value, was approximated by measuring when thread overhead stops mattering
    if(width * max_y > 200000){
        int thread_amount = getCoreAmount();

        pthread_t threads[thread_amount];
        thread_data data[thread_amount];

        for (int i = 0; i < thread_amount; i++) {
            pthread_t thread;
            // i == bottom_y, max_y = top y, threat_amount = step size
            data[i] = (thread_data) {start, width, height, i, max_y, thread_amount, res, n, color, img};;
            pthread_create(&thread, NULL, tricornThreading, &data[i]);
            threads[i] = thread;
        }

        for (int i = 0; i < thread_amount; i++) {
            pthread_join(threads[i], NULL);
        }
    } else {
        tricorn_V4(start, width, height, res, n, color, img);
    }
}

/* void tricornThreadPlot(float complex start, size_t width, ssize_t height, float res, unsigned n, bool color,
    unsigned char* img, int thread_amount) {
    size_t max_y = height > 0 ? height : -height;
    if (thread_amount < 1) {
        fprintf(stderr, "Threads cant be smaller than 1");
    }

    //  The 200k Value, was approximated by measuring when thread overhead stops mattering
    if(width * max_y > 200000){
        pthread_t threads[thread_amount];
        thread_data data[thread_amount];

        for (int i = 0; i < thread_amount; i++) {
            pthread_t thread;
            // i == bottom_y, max_y = top y, threat_amount = step size
            data[i] = (thread_data) {start, width, height, i, max_y, thread_amount, res, n, color, img};;
            pthread_create(&thread, NULL, tricornThreading, &data[i]);
            threads[i] = thread;
        }

        for (int i = 0; i < thread_amount; i++) {
            pthread_join(threads[i], NULL);
        }
    } else {
        tricorn_V5(start, width, height, res, n, color, img);
    }
} */
