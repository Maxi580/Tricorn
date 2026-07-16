#include <complex.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "bitmap.h"
#include <immintrin.h>
#include <time.h>

#include "NaiveImplementation/tricorn_V1.h"
#include "Simd/tricorn_V2.h"
#include "Circle/tricorn_V3.h"
#include "LatencyThroughput/tricorn_V4.h"
#include "MultiThreading/tricorn.h"
#include "utils.h"
#include "testSuite.h"


double performanceTest(void (*fn)(float complex, size_t, ssize_t, float,  unsigned, bool, unsigned char *),
                        TestCase tc, size_t iterations) {
    /* Misst die Zeit eines übergebenen Testcases so oft wie in iterations angegeben bzw erhöht diese bis der
     * Test valide ist.*/
    // Speicher allozieren
    int bytesPerPixel = tc.color ? 3 : 1;
    size_t height = abs_size(tc.height_signed);
    size_t row_size = ((tc.width * bytesPerPixel+3)/4)*4;
    size_t pixelArraySize = row_size * height;
    unsigned char *img = malloc(pixelArraySize);
    if (img == NULL) {
        fprintf(stderr, "Error allocating memory\n");
        exit(EXIT_FAILURE);
    }

    struct timespec time_start, time_end;
    double time = 0;
    bool test_is_valid = false;

    while (!test_is_valid) {
        clock_gettime(CLOCK_MONOTONIC, &time_start);
        for (size_t i = 0; i < iterations; i++) {
            fn(tc.start, tc.width, tc.height_signed, tc.res, tc.n, tc.color, img);
        }
        clock_gettime(CLOCK_MONOTONIC, &time_end);

        time = (double)(time_end.tv_sec - time_start.tv_sec) + 1e-9 * (time_end.tv_nsec - time_start.tv_nsec);
        if (time < MIN_TEST_DURATION) { // unter 1 Sekunde getestet (ungültig) → erhöhe iterationen
            size_t increased_iterations = (time > 0.0) ?
                            (size_t) ((double) iterations * (1.3 * MIN_TEST_DURATION/time)) : 0;
            iterations = (increased_iterations > iterations) ? increased_iterations : (iterations + 1);
        } else {
            test_is_valid = true;
        }
    }

    free(img);
    return time / (double) iterations;
}

void printDebug(unsigned char *base, unsigned char *test, size_t pixel_array_size) {
    for (size_t i = 0; i < pixel_array_size; i++) {
        if (base[i] != test[i]) {
            printf("Erster Unterschied bei Byte %zu: %u vs %u\n", i, base[i], test[i]);
            break;
        }
    }
}

bool correctnessTest(TestCase tc, bool print_debug) {
    // Vergleicht die Ausgaben aller optimierten Implementierungen mit der naiven Implementierung
    bool all_passed = true;

    // Speicher allozieren
    int bytesPerPixel = tc.color ? 3 : 1;
    size_t height = abs_size(tc.height_signed);
    size_t row_size = ((tc.width * bytesPerPixel+3)/4)*4;
    size_t pixel_array_size = row_size * height;
    unsigned char *v0 = malloc(pixel_array_size);
    unsigned char *v1 = malloc(pixel_array_size);
    if (v0 == NULL || v1 == NULL) {
        fprintf(stderr, "Error allocating memory\n");
        free(v0);
        free(v1);
        exit(EXIT_FAILURE);
    }
    // Naive Implementierung
    tricorn_V1(tc.start, tc.width, tc.height_signed, tc.res, tc.n, tc.color, v0);

    tricorn_V2(tc.start, tc.width, tc.height_signed, tc.res, tc.n, tc.color, v1);
    if (memcmp(v0, v1, pixel_array_size)) {
        if (print_debug) {
            printf("SIMD Error\n");
            printDebug(v0, v1, pixel_array_size);
        }
        all_passed = false;
    }
    // v1 mit auffälligen Werten füllen, damit nicht beschriebende Bereiche bemerkt werden
    memset(v1, 0xAA, pixel_array_size);

    tricorn_V3(tc.start, tc.width, tc.height_signed, tc.res, tc.n, tc.color, v1);
    if (memcmp(v0, v1, pixel_array_size)) {
        if (print_debug) {
            printf("Circle Version Error\n");
            printDebug(v0, v1, pixel_array_size);
        }
        all_passed = false;
    }
    memset(v1, 0xAA, pixel_array_size);

    tricorn_V4(tc.start, tc.width, tc.height_signed, tc.res, tc.n, tc.color, v1);
    if (memcmp(v0, v1, pixel_array_size)) {
        if (print_debug) {
            printf("Latency Throughput Version Error\n");
            printDebug(v0, v1, pixel_array_size);
        }
        all_passed = false;
    }
    memset(v1, 0xAA, pixel_array_size);

    tricorn(tc.start, tc.width, tc.height_signed, tc.res, tc.n, tc.color, v1);
    if (memcmp(v0, v1, pixel_array_size) != 0) {
        if (print_debug) {
            printf("Multithreading Version Error\n");
            printDebug(v0, v1, pixel_array_size);
        }
        all_passed = false;
    }

    free(v0);
    free(v1);
    return all_passed;
}

bool individualTests(bool print_debug) {
    unsigned iterations = 100;
    bool all_passed = true;

    TestCase test_cases[] = {
        {-2.0 + 1.5f * I, 800, -600, 0.005f, 100, false, "topDownGray"},
        {-2.0 + 1.5f * I, 800, -600, 0.005f, 100, true, "topDownColor"},
        {-2.0 + 1.5f * I, 1, 1, 0.005f, 100, false, "topDownGray2"},
        {-2.0 - 1.5f * I, 800, +600, 0.005f, 100, false, "BottomUpGray"},
        {-2.0 - 2.0f * I, 800, +600, 0.005f, 100, false, "BottomUpColor"},
        {-2.0 - 1.5f * I, 800, +600, 0.005f, 100, false, "BottomUpGray2"},

        {-0.5 + 2.0f * I, 200, -200, 0.01f, 50, false, "randomGrayscale"},
        {-2.0 + 1.5f * I, 800, 600, 0.005f, 100, false, "OutOfSCope"},

        {-2.0 + 2.0f * I, 1, -1, 0.01f, 10, false, "1x1 topDown"},
        {-2.0 - 2.0f * I, 2, 2, 0.01f, 10, false, "2x2 BottomUp"},

        {-2.0 + 2.0f * I, 3, -3, 0.01f, 10, false, "3x3 topDown"},
        {-2.0 + 2.0f * I, 3, -300, 0.01f, 10, false, "3x300 topDown"},

        {-0.5 + 0.5 * I, 300, -200, 0.001f, 1500, false, "high iterations"},

        {-2.0 -2.0 * I, 300, 200, 0.000001f, 10, false, "zoomed in"},
        {-2.0 -2.0 * I, 300, 200, 1.0f, 10, false, "zoomed out"},

        {-2.0 -2.0 * I, 401, 200, 0.005f, 10, false, "Breite mod 4 = 1"},
        {-2.0 -2.0 * I, 402, 200, 0.005f, 10, false, "Breite mod 4 = 2"},
        {-2.0 -2.0 * I, 403, 200, 0.005f, 10, false, "Breite mod 4 = 3"},

        {-2.0 + -2.0f * I, 447, 447, 0.005f, 100, false, "mt width * height < 200k"},
        {-2.0 + -2.0f * I, 448, 448, 0.005f, 100, false, "mt width * height > 200k"},
    };

    size_t test_case_count = sizeof(test_cases) / sizeof(test_cases[0]);
    size_t passed = 0;

    for (size_t i = 0; i < test_case_count; i++) {
        printf("Test %zu/%zu: %s\n", i+1, test_case_count, test_cases[i].name);
        printf("Start:\t\t%f + %2fi\n", crealf(test_cases[i].start), cimagf(test_cases[i].start));
        printf("Breite:\t\t%zu\n", test_cases[i].width);
        printf("Höhe:\t\t%zd\n", test_cases[i].height_signed);

        printf("Auflösung\t%f\n", test_cases[i].res);
        printf("Iterationen n:\t%u\n", test_cases[i].n);
        printf("Farbe:\t\t%s\n", test_cases[i].color ? "true" : "false");
        bool correct = correctnessTest(test_cases[i], print_debug);

        if (correct) {
            double time = performanceTest(tricorn, test_cases[i], iterations);
            double time_V1 = performanceTest(tricorn_V1, test_cases[i], iterations);
            double time_V2 = performanceTest(tricorn_V2, test_cases[i], iterations);
            double time_V3 = performanceTest(tricorn_V3, test_cases[i], iterations);
            double time_V4 = performanceTest(tricorn_V4, test_cases[i], iterations);
            printf("BESTANDEN: Zeiten (pro Iteration) - "
                   "V0: %f V1: %f V2: %f V3: %f V4: %f\n\n", time, time_V1, time_V2, time_V3, time_V4);
            passed++;
        } else {
            printf("UNGÜLTIGER TEST\n\n");
            all_passed = false;
        }
    }
    printf("Manuelle Tests %zu/%zu bestanden.\n\n", passed, test_case_count);
    return all_passed;
}

bool randomizedTest(size_t width, ssize_t height_signed, int trials){
    // Führt viele randomisierte tests aus, variiert startwerte und resolution
    printf("Starte bruteforce Tests\n\n");
    bool all_passed = true;
    int passed = 0;

    float max = 2.0f;
    float min = -2.0f;
    unsigned n = 256;
    bool color = false;
    for (int iterations = 0; iterations < trials; iterations++) {
        float start_real = min + (max - min) * ((float) rand()/ (float) RAND_MAX);
        float start_imag = min + (max - min) * ((float) rand()/ (float) RAND_MAX);
        float complex start = start_real + start_imag * I;
        float res = 0.0001f + 100.0f * ((float) rand()/ (float) RAND_MAX);

        TestCase tc = {start, width, height_signed, res, n, color, "bruteforce"};
        if (correctnessTest(tc, false)) {
            passed++;
        } else {
            all_passed = false;
        }
    }
    printf("Bruteforce Tests sind fertig. Erfolg bei %d/%d\n", passed, trials);
    return all_passed;
}

bool symmetryTest(size_t width, ssize_t height_signed, int trials) {
    /* Führt viele randomisierte Tests aus, die gezielt das center auf einem geraden y setzen, da dann symmetrie
     * funktionieren muss. Dann wählen wir zufällig sehr kleine resolutions, da dabei unter Symmetrie immer
     * Probleme aufgetreten sind*/
    printf("Starte Symmetrie Tests\n\n");
    bool all_passed = true;
    int passed = 0;

    // Betrag der Bildhöhe bestimmen und Vorzeichen für die Bildausrichtung in 'factor' speichern
    size_t height = abs_size(height_signed);
    float factor = height_signed < 0 ? -1.0f : 1.0f;

    unsigned n = 100;
    bool color = false;

    srand(3141592653);
    for (int i = 0; i < trials;  i++){
        int center = height / 2 + rand() % 10;

        // Kleine zufällige Auflösung wählen. Somit entstehen Rundungsfehler,
        // sodass gespiegelte Koordinaten nicht den exakt gleichen Imaginärteil haben
        float res = 0.002f + 0.011f * ((float) rand()/(float) RAND_MAX);

        // Zufälligen Startwert für den Realteil wählen.
        float start_real = -2.0f + 4.0f * ((float) rand()/ (float) RAND_MAX);

        // Imaginärteil so berechnen, dass der durch 'center' gewählte
        // Mittelpunkt auf der Symmetrieachse liegt
        float start_imag = -(center * factor * res);

        float complex start = start_real + start_imag * I;

        TestCase tc = {start, width, height_signed, res, n, color, "symmetrie"};
        if (correctnessTest(tc, false)) {
            passed++;
        } else {
            all_passed = false;
        }
    }
    printf("Symmetrie Tests sind fertig. Erfolg bei %d/%d\n", passed, trials);
    return all_passed;
}

bool testSuite() {
    printf("Starte Tests:\n");
    individualTests(true);
    randomizedTest(200, 200, 100);
    symmetryTest(200, 200, 100);
    printf("Tests beendet:\n");
    return true;
}
