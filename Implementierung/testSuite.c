#include <stdio.h>
#include <complex.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

#include "NaiveImplementation/image.h"
#include "Simd/image.h"
#include "Sym/image.h"
#include "Circle/image.h"
#include "LatencyThroughput/image.h"
#include "MultiThreading/image.h"
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

    bool isAllocted = false;
    if (img == NULL) {
        isAllocted = true;

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

    if (isAllocted) {
        free(img);
    }

    return time / (double)benchmarkIterations;;
}

timeMeasurement* plotTableGenerationAgainstSizeAndRes(size_t start_length, size_t steps, size_t step_size,
                    const unsigned n, const bool color) {
    printf("Starting test against changing width and scale res:\n");
    timeMeasurement* results = malloc(sizeof(timeMeasurement) * steps);
    if (results == NULL) {
        printf("malloc() returned NULL\n");
        return NULL;
    }
    const float complex start_value = -2.0f -2.0f * I;

    for (size_t step = 0; step < steps; step++) {
        size_t length = start_length + step * step_size;
        const float res = 4.0f / (float) length;
        results[step].width = length;
        results[step].iterations = n;
        results[step].res = res;
        results[step].naive = measureTime(tricornNaive, start_value, length, (ssize_t)length, res, n, color, NULL);
        results[step].simd = measureTime(tricornSimd, start_value, length, (ssize_t)length, res, n, color, NULL);
        results[step].sym = measureTime(tricornSym, start_value, length, (ssize_t)length, res, n, color, NULL);
        results[step].circle = measureTime(tricornCircle, start_value, length, (ssize_t)length, res, n, color, NULL);
        results[step].latency = measureTime(tricornLatencyThroughput, start_value, length, (ssize_t)length, res, n, color, NULL);
        results[step].multithreading = measureTime(tricornMultiThreading, start_value, length, (ssize_t)length, res, n, color, NULL);
    }
    return results;
}

timeMeasurement* plotTableGenerationAgainstIterationsBlack(const size_t length, const size_t steps, const size_t step_size, const float res,
                    const unsigned start_n, const bool color) {
    printf("Starting test against changing iterations in black of fractale (Start: 0+0i):\n");
    timeMeasurement* results = malloc(sizeof(timeMeasurement) * steps);
    if (results == NULL) {
        printf("malloc() returned NULL\n");
        return NULL;
    }
    const float complex start_value = 0.0;

    for (size_t step = 0; step < steps; step++) {
        unsigned n = start_n + step * step_size;
        results[step].width = length;
        results[step].iterations = n;
        results[step].res = res;
        results[step].naive = measureTime(tricornNaive, start_value, length, (ssize_t)length, res, n, color, NULL);
        results[step].simd = measureTime(tricornSimd, start_value, length, (ssize_t)length, res, n, color, NULL);
        results[step].sym = measureTime(tricornSym, start_value, length, (ssize_t)length, res, n, color, NULL);
        results[step].circle = measureTime(tricornCircle, start_value, length, (ssize_t)length, res, n, color, NULL);
        results[step].latency = measureTime(tricornLatencyThroughput, start_value, length, (ssize_t)length, res, n, color, NULL);
        results[step].multithreading = measureTime(tricornMultiThreading, start_value, length, (ssize_t)length, res, n, color, NULL);
    }
    return results;
}

timeMeasurement* plotTableGenerationAgainstIterationsWhite(const size_t length, const size_t steps, const size_t step_size, const float res,
                    const unsigned start_n, const bool color) {
    printf("Starting test against changing iterations in empty space (Start: 3+3i):\n");
    timeMeasurement* results = malloc(sizeof(timeMeasurement) * steps);
    if (results == NULL) {
        printf("malloc() returned NULL\n");
        return NULL;
    }
    const float complex start_value = 3.0 + 3.0 * I;

    for (size_t step = 0; step < steps; step++) {
        unsigned n = start_n + step * step_size;
        results[step].width = length;
        results[step].iterations = n;
        results[step].res = res;
        results[step].naive = measureTime(tricornNaive, start_value, length, (ssize_t)length, res, n, color, NULL);
        results[step].simd = measureTime(tricornSimd, start_value, length, (ssize_t)length, res, n, color, NULL);
        results[step].sym = measureTime(tricornSym, start_value, length, (ssize_t)length, res, n, color, NULL);
        results[step].circle = measureTime(tricornCircle, start_value, length, (ssize_t)length, res, n, color, NULL);
        results[step].latency = measureTime(tricornLatencyThroughput, start_value, length, (ssize_t)length, res, n, color, NULL);
        results[step].multithreading = measureTime(tricornMultiThreading, start_value, length, (ssize_t)length, res, n, color, NULL);
    }
    return results;
}

timeMeasurement* plotTableGenerationAgainstIterationsEdge(const size_t length, const size_t steps, const size_t step_size, const float res,
                    const unsigned start_n, const bool color) {
    printf("Starting test against changing iterations fractale edge (Start: -1+0i):\n");
    timeMeasurement* results = malloc(sizeof(timeMeasurement) * steps);
    if (results == NULL) {
        printf("malloc() returned NULL\n");
        return NULL;
    }
    float complex start_value = -1.0 + 0.0 * I;

    for (size_t step = 0; step < steps; step++) {
        unsigned n = start_n + step * step_size;
        results[step].width = length;
        results[step].iterations = n;
        results[step].res = res;
        results[step].naive = measureTime(tricornNaive, start_value, length, (ssize_t)length, res, n, color, NULL);
        results[step].simd = measureTime(tricornSimd, start_value, length, (ssize_t)length, res, n, color, NULL);
        results[step].sym = measureTime(tricornSym, start_value, length, (ssize_t)length, res, n, color, NULL);
        results[step].circle = measureTime(tricornCircle, start_value, length, (ssize_t)length, res, n, color, NULL);
        results[step].latency = measureTime(tricornLatencyThroughput, start_value, length, (ssize_t)length, res, n, color, NULL);
        results[step].multithreading = measureTime(tricornMultiThreading, start_value, length, (ssize_t)length, res, n, color, NULL);
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

    timeMeasurement* time_table_size_res = plotTableGenerationAgainstSizeAndRes(50, 100, 50, 256, false);
    printTable(time_table_size_res, 100);
    free(time_table_size_res);

    timeMeasurement* time_table_iterations_black = plotTableGenerationAgainstIterationsBlack(800, 100, 20, 0.0001f, 20, false);
    printTable(time_table_iterations_black, 100);
    free(time_table_iterations_black);

    timeMeasurement* time_table_iterations_white = plotTableGenerationAgainstIterationsWhite(800, 100, 20, 0.0001f, 20, false);
    printTable(time_table_iterations_white, 100);
    free(time_table_iterations_white);

    timeMeasurement* time_table_iterations_edge = plotTableGenerationAgainstIterationsEdge(800, 100, 20, 0.001f, 20, false);
    printTable(time_table_iterations_edge, 100);
    free(time_table_iterations_edge);

    printf("\nPerformance Test für Graphen Fertig\n");
    return 0;
}

int testSuite() {
    printf("Starte Performance Tests:\n");

    struct TestCase testCases[] = {
        {-2.0 +1.5f * I, 800, -600, 0.005f, 100 ,false, "topDownGray"},
        {-2.0 -1.5f * I, 800, +600, 0.005f, 100 ,false, "BottomUpGray"},
        {-2.0 +1.5f * I, 800, -600, 0.005f, 100 ,false, "TopDownGray"},
        {-2.0 -1.5f * I, 800, +600, 0.005f, 100 ,false, "BottomUpGray"},

        {-0.5 +2.0f * I, 200, -200, 0.01f, 50 ,false, "randomGrayscale"},
        {-2.0 +1.5f * I, 800, 600, 0.005f, 100 ,false, "OutOfSCope"},

        {-2.0 +2.0f * I, 1, -1, 0.01f, 10 ,false, "1x1 topDown"},
        {-2.0 -2.0f * I, 2, 2, 0.01f, 10 ,false, "2x2 BottomUp"},

        {-2.0 +2.0f * I, 3, -3, 0.01f, 10 ,false, "3x3 topDown"},
        {-2.0 +2.0f * I, 3, -300, 0.01f, 10 ,false, "3x300 "},

        {-0.5 +0.5 * I, 300, -200, 0.001f, 1500 ,false, "high iterations "},
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
                case 0: avg_time = measureTime(tricornNaive, tc.start, tc.width, tc.height, tc.res, tc.n, tc.color, imgNaive); break;
                case 1: avg_time = measureTime(tricornSimd, tc.start, tc.width, tc.height, tc.res, tc.n, tc.color, img); break;
                case 2: avg_time = measureTime(tricornSym, tc.start, tc.width, tc.height, tc.res, tc.n, tc.color, img); break;
                case 3: avg_time = measureTime(tricornCircle, tc.start, tc.width, tc.height, tc.res, tc.n, tc.color, img); break;
                case 4: avg_time = measureTime(tricornLatencyThroughput, tc.start, tc.width, tc.height, tc.res, tc.n, tc.color, img); break;
                default: avg_time = measureTime(tricornMultiThreading, tc.start, tc.width, tc.height, tc.res, tc.n, tc.color, img); break;
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