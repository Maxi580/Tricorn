#include <stdio.h>
#include <complex.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

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

double measureTime(void (*fn)(float complex, size_t, ssize_t, float,  unsigned, bool, unsigned char *),
                    const float complex start, const size_t width, const ssize_t height_signed, const float res, const unsigned n, const bool color) {
    size_t height;
    if (height_signed < 0) {
        height = (size_t)(-height_signed);
    }
    else {
        height = (size_t)height_signed ;
    }
    const size_t row_size = ((width * 3+3)/4)*4;
    const size_t pixelArraySize = row_size * height;

    unsigned char *img = malloc(pixelArraySize);
    if (img == NULL) {
        printf("malloc() returned NULL\n");
        return -1.0;
    }

    struct timespec time_start;
    clock_gettime(CLOCK_MONOTONIC, &time_start);

    fn(start, width, height_signed, res, n, color, img);

    struct timespec time_end;
    clock_gettime(CLOCK_MONOTONIC, &time_end);
    const double time = (double)(time_end.tv_sec - time_start.tv_sec) + 1e-9 * (time_end.tv_nsec - time_start.tv_nsec);

    free(img);
    return time;
}

timeMeasurement* plotTableGenerationAgainstSizeAndRes(const size_t start_length, const size_t steps, const size_t step_size,
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
        results[step].naive = measureTime(tricornNaive, start_value, length, (ssize_t)length, res, n, color);
        results[step].simd = measureTime(tricornSimd, start_value, length, (ssize_t)length, res, n, color);
        results[step].sym = measureTime(tricornSym, start_value, length, (ssize_t)length, res, n, color);
        results[step].circle = measureTime(tricornCircle, start_value, length, (ssize_t)length, res, n, color);
        results[step].latency = measureTime(tricornLatencyThroughput, start_value, length, (ssize_t)length, res, n, color);
        results[step].multithreading = measureTime(tricornMultiThreading, start_value, length, (ssize_t)length, res, n, color);
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
        results[step].naive = measureTime(tricornNaive, start_value, length, (ssize_t)length, res, n, color);
        results[step].simd = measureTime(tricornSimd, start_value, length, (ssize_t)length, res, n, color);
        results[step].sym = measureTime(tricornSym, start_value, length, (ssize_t)length, res, n, color);
        results[step].circle = measureTime(tricornCircle, start_value, length, (ssize_t)length, res, n, color);
        results[step].latency = measureTime(tricornLatencyThroughput, start_value, length, (ssize_t)length, res, n, color);
        results[step].multithreading = measureTime(tricornMultiThreading, start_value, length, (ssize_t)length, res, n, color);
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
        results[step].naive = measureTime(tricornNaive, start_value, length, (ssize_t)length, res, n, color);
        results[step].simd = measureTime(tricornSimd, start_value, length, (ssize_t)length, res, n, color);
        results[step].sym = measureTime(tricornSym, start_value, length, (ssize_t)length, res, n, color);
        results[step].circle = measureTime(tricornCircle, start_value, length, (ssize_t)length, res, n, color);
        results[step].latency = measureTime(tricornLatencyThroughput, start_value, length, (ssize_t)length, res, n, color);
        results[step].multithreading = measureTime(tricornMultiThreading, start_value, length, (ssize_t)length, res, n, color);
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
        results[step].naive = measureTime(tricornNaive, start_value, length, (ssize_t)length, res, n, color);
        results[step].simd = measureTime(tricornSimd, start_value, length, (ssize_t)length, res, n, color);
        results[step].sym = measureTime(tricornSym, start_value, length, (ssize_t)length, res, n, color);
        results[step].circle = measureTime(tricornCircle, start_value, length, (ssize_t)length, res, n, color);
        results[step].latency = measureTime(tricornLatencyThroughput, start_value, length, (ssize_t)length, res, n, color);
        results[step].multithreading = measureTime(tricornMultiThreading, start_value, length, (ssize_t)length, res, n, color);
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

int testSuite() {
    printf("Starte Performance tests:\n");

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

    printf("\nTest Suite Fertig\n");
    return 0;
}

// Für neue Testfällen einfach neuen testCase() in testSuite() hinzufügen

// Bei neuen Implementationen/Versionen in testAllVersions() das Schleifenlimit
// erhöhen und die neue Version als if else hinzufügen