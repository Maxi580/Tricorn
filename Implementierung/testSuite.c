#include <stdio.h>
#include <complex.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

#include "NaiveImplementation/tricorn_V1.h"
#include "Simd/tricorn_V2.h"
#include "Sym/tricorn_V3.h"
#include "Circle/tricorn_V4.h"
#include "LatencyThroughput/tricorn_V5.h"
#include "MultiThreading/tricorn.h"
#include "testSuite.h"

typedef struct {
    double naive;
    double simd;
    double sym;
    double circle;
    double latency;
    double multithreading;
    size_t width;
    unsigned iterations;
    float res;
} timeMeasurement;

struct TestCase {
    float complex start;
    size_t width;
    ssize_t height;
    float res;
    unsigned n;
    bool color;
    const char *name;
};

double measureTime(void (*fn)(float complex, size_t, ssize_t, float,  unsigned, bool, unsigned char *),
                    const float complex start, const size_t width, const ssize_t height_signed, const float res, const unsigned n, const bool color, unsigned char *img) {

    bool is_allocated = false;
    if (img == NULL) {
        is_allocated = true;

        size_t height;
        if (height_signed < 0) {
            height = (size_t)(-height_signed);
        }
        else {
            height = (size_t)height_signed ;
        }
        const size_t row_size = ((width * 3+3)/4)*4;
        const size_t pixelArraySize = row_size * height;

        img = malloc(pixelArraySize);
        if (img == NULL) {
            printf("malloc() returned NULL\n");
            return -1.0;
        }
    }


    size_t benchmarkIterations = 3;
    struct timespec time_start, time_end;
    double time = 0;
    bool isValid = false;

    while (!isValid) {
        clock_gettime(CLOCK_MONOTONIC, &time_start);
        for (size_t i = 0; i < benchmarkIterations; i++) {
            fn(start, width, height_signed, res, n, color, img);
        }
        clock_gettime(CLOCK_MONOTONIC, &time_end);

        time = (double)(time_end.tv_sec - time_start.tv_sec) + 1e-9 * (time_end.tv_nsec - time_start.tv_nsec);
        if (time < 1.0) { // unter 1 Sekunde getestet (ungültig) → erhöhe iterationen
            size_t calcBenchmarkIterations = (time > 0.0) ? (size_t) ((double)benchmarkIterations * (1.3/time)) : 0;
            benchmarkIterations = (calcBenchmarkIterations > benchmarkIterations) ? calcBenchmarkIterations : (benchmarkIterations + 1);
        } else {
            isValid = true;
        }
    }

    if (is_allocated) {
        free(img);
    }

    return time / (double)benchmarkIterations;;
}

timeMeasurement* plotTableGeneration(const size_t steps, const float complex start_value, const size_t start_length,  const size_t step_length,
                    const size_t start_n, const size_t step_n, const float start_res, const bool color) {
    timeMeasurement* results = malloc(sizeof(timeMeasurement) * steps);
    if (results == NULL) {
        printf("malloc() returned NULL\n");
        return NULL;
    }

    for (size_t step = 0; step < steps; step++) {
        const size_t length = start_length + step * step_length;
        const size_t n = start_n + step * step_n;

        const float res = (start_res == 0.0) ? (4.0f / (float) length) : start_res;

        results[step].width = length;
        results[step].iterations = n;
        results[step].res = res;

        results[step].naive = measureTime(tricorn_V1, start_value, length, (ssize_t)length, res, n, color, NULL);
        results[step].simd = measureTime(tricorn_V2, start_value, length, (ssize_t)length, res, n, color, NULL);
        results[step].sym = measureTime(tricorn_V3, start_value, length, (ssize_t)length, res, n, color, NULL);
        results[step].circle = measureTime(tricorn_V4, start_value, length, (ssize_t)length, res, n, color, NULL);
        results[step].latency = measureTime(tricorn_V5, start_value, length, (ssize_t)length, res, n, color, NULL);
        results[step].multithreading = measureTime(tricorn, start_value, length, (ssize_t)length, res, n, color, NULL);
    }
    return results;
}

void printTable(const timeMeasurement* r, size_t steps) {
    if (r == NULL) {
        printf("Passed Pointer was NULL");
        return;
    }

    printf("%12s %12s %12s %12s %12s %12s %12s %12s %12s\n",
           "width", "iter", "res", "naive", "simd", "sym", "circle", "latency", "multithr");
    for (size_t i = 0; i < steps; i++) {
        printf("%12zu %12u %12f %12f %12f %12f %12f %12f %12f\n",
               r[i].width, r[i].iterations, r[i].res, r[i].naive, r[i].simd, r[i].sym,
               r[i].circle, r[i].latency, r[i].multithreading);
    }
}

int genGraphTable() {
    printf("Starte Performance Test für Graphen:\n");

    printf("Starting test against changing width and scale res:\n");
    timeMeasurement* time_table_size_res = plotTableGeneration(100,  (-2.0f -2.0f * I), 50,  50,
                    256, 0, 0, false);
    printTable(time_table_size_res, 100);
    free(time_table_size_res);

    printf("Starting test against changing iterations in black of fractale (Start: 0+0i):\n");
    timeMeasurement* time_table_iterations_black = plotTableGeneration(70, 0.0, 800,  0,
                    50, 50, 0.0001f, false);
    printTable(time_table_iterations_black, 70);
    free(time_table_iterations_black);

    printf("Starting test against changing iterations in empty space (Start: 3+3i):\n");
    timeMeasurement* time_table_iterations_white = plotTableGeneration(70, (3.0 + 3.0 * I), 800,  0,
                    50, 50, 0.0001f, false);
    printTable(time_table_iterations_white, 70);
    free(time_table_iterations_white);

    printf("Starting test against changing iterations fractale edge (Start: -1+0i):\n");
    timeMeasurement* time_table_iterations_edge = plotTableGeneration(70, (-1.0 + 0.0 * I), 800, 0,
                    50, 50, 0.001f, false);
    printTable(time_table_iterations_edge, 70);
    free(time_table_iterations_edge);

    printf("\nPerformance Test für Graphen Fertig\n");
    return 0;
}

int testSuite() {
    printf("Starte Performance Tests:\n");

    const struct TestCase testCases[] = {
        {-2.0 +1.5f * I, 800, -600, 0.005f, 100 ,false, "topDownGray"},
        {-2.0 -1.5f * I, 800, +600, 0.005f, 100 ,false, "BottomUpGray"},
        {-2.0 +1.5f * I, 800, -600, 0.005f, 100 ,false, "TopDownGray"},
        {-2.0 -1.5f * I, 800, +600, 0.005f, 100 ,false, "BottomUpGray"},

        {-0.5 +2.0f * I, 200, -200, 0.01f, 50 ,false, "randomGrayscale"},
        {-2.0 +1.5f * I, 800, 600, 0.005f, 100 ,false, "OutOfSCope"},

        {-2.0 +2.0f * I, 1, -1, 0.01f, 10 ,false, "1x1 topDown"},
        {-2.0 -2.0f * I, 2, 2, 0.01f, 10 ,false, "2x2 BottomUp"},

        {-2.0 +2.0f * I, 3, -3, 0.01f, 10 ,false, "3x3 topDown"},
        {-2.0 +2.0f * I, 3, -300, 0.01f, 10 ,false, "3x300"},

        {-0.5 +0.5 * I, 300, -200, 0.001f, 1500 ,false, "high iterations"},
    };

    size_t testCasesCount = sizeof(testCases)/sizeof(testCases[0]);

    for (size_t i = 0; i < testCasesCount; i++) {
        const struct TestCase tc = testCases[i];
        printf("\nTest %zu/%zu: \t%s\n", i+1, testCasesCount, tc.name);
        printf("Dimension:\t%zu x %zd\n",tc.width, tc.height);
        printf("Start:\t\t%f + %fi\n", crealf(tc.start), cimagf(tc.start));
        printf("Auflösung:\t%f\n", tc.res);
        printf("Iterationen n:\t%u\n", tc.n);
        printf("Farbe:\t\t%s\n", tc.color ? "true" : "false");

        printf("\nDauer pro Iteration:\n");

        size_t height;
        if (tc.height < 0) {
            height = (size_t)(-tc.height);
        }
        else {
            height = (size_t)tc.height ;
        }

        const size_t row_size = ((tc.width * 3+3)/4)*4;
        const size_t pixelArraySize = row_size * height;

        unsigned char *imgNaive = malloc(pixelArraySize);
        unsigned char *img = malloc(pixelArraySize);
        if (img == NULL || imgNaive == NULL) { //Fehler
            printf("Error allocating memory\n");
            free(imgNaive);
            free(img);
            return 1;
        }

        for (size_t v = 0; v < 6; v++) {
            double avg_time = 0;
            switch (v) {
                case 0: avg_time = measureTime(tricorn_V1, tc.start, tc.width, tc.height, tc.res, tc.n, tc.color, imgNaive); break;
                case 1: avg_time = measureTime(tricorn_V2, tc.start, tc.width, tc.height, tc.res, tc.n, tc.color, img); break;
                case 2: avg_time = measureTime(tricorn_V3, tc.start, tc.width, tc.height, tc.res, tc.n, tc.color, img); break;
                case 3: avg_time = measureTime(tricorn_V4, tc.start, tc.width, tc.height, tc.res, tc.n, tc.color, img); break;
                case 4: avg_time = measureTime(tricorn_V5, tc.start, tc.width, tc.height, tc.res, tc.n, tc.color, img); break;
                default: avg_time = measureTime(tricorn, tc.start, tc.width, tc.height, tc.res, tc.n, tc.color, img); break;
            }
            printf("V%zu: %gs\n", v, avg_time);

            if (v != 0) { // Naive Implementation (v0) mit den anderen Versionen vergleichen
                if (memcmp(imgNaive, img, pixelArraySize) != 0) {
                    for (size_t j = 0; j < pixelArraySize; j++) {
                        if (imgNaive[j] != img[j]) {
                            printf("Error: Bitmap unterschiedlich v0/v%zu bei Byte %zu: %u vs %u\n", v, j, imgNaive[j], img[j]);
                            break;
                        }
                    }
                }
            }
        }

        free(imgNaive);
        free(img);

    }
    printf("\nTest Suite Fertig\n");
    return 0;
}

// Schwarz: ./main -s -0 -0 -d 800 800 -n 100 -r 0.0001 -o test -V 4
// Weiß: ./main -s 3 3 -d 800 800 -n 100 -r 0.0001 -o test -V 4
// Kante vom Fraktal: ./main -s -1 0 -d 800 800 -n 100 -r 0.0001 -o test -V 4