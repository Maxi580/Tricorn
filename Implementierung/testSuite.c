#include <stdio.h>
#include <complex.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#include "bitmap.h"
#include "NaiveImplementation/image.h"
#include "Simd/image.h"
#include "Simd/escapetime.h"
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
} timeMeasurement;

double measureTime(void (*fn)(float complex, size_t, ssize_t, float,  unsigned, bool, unsigned char *),
                    float complex start, size_t width, ssize_t height_signed, float res, unsigned n, bool color) {
    size_t height;
    if (height_signed < 0) {
        height = (size_t)(-height_signed);
    }
    else {
        height = (size_t)height_signed ;
    }
    size_t row_size = ((width * 3+3)/4)*4;
    size_t pixelArraySize = row_size * height;

    unsigned char *img = malloc(pixelArraySize);
    if (img == NULL) {
        printf("malloc() returned NULL\n");
        return false;
    }

    struct timespec time_start;
    clock_gettime(CLOCK_MONOTONIC, &time_start);

    fn(start, width, height_signed, res, n, color, img);

    struct timespec time_end;
    clock_gettime(CLOCK_MONOTONIC, &time_end);
    double time = time_end.tv_sec - time_start.tv_sec + 1e-9 * (time_end.tv_nsec - time_start.tv_nsec);

    free(img);
    return time;
}


timeMeasurement* measureAgainstSize(size_t start_length, size_t steps, size_t step_size, const float res,
                    const unsigned n, const bool color) {
    printf("Starting test against changing width:\n");
    timeMeasurement* results = malloc(sizeof(timeMeasurement) * steps);
    if (results == NULL) {
        printf("malloc() returned NULL\n");
        return NULL;
    }
    float complex start_value = -2.0f -2.0f * I;

    for (size_t step = 0; step < steps; step++) {
        size_t length = start_length + step * step_size;
        results[step].width = length;
        results[step].naive = measureTime(tricornNaive, start_value, length, length, res, n, color);
        results[step].simd = measureTime(tricornSimd, start_value, length, length, res, n, color);
        results[step].sym = measureTime(tricornSym, start_value, length, length, res, n, color);
        results[step].circle = measureTime(tricornCircle, start_value, length, length, res, n, color);
        results[step].latency = measureTime(tricornLatencyThroughput, start_value, length, length, res, n, color);
        results[step].multithreading = measureTime(tricornMultiThreading, start_value, length, length, res, n, color);
    }
    return results;
}

void printTable(const timeMeasurement* r, size_t steps) {
    if (r == NULL) {
        printf("Passed Pointer was NULL");
        return;
    }

    printf("%12s %12s %12s %12s %12s %12s %12s\n",
           "width", "naive", "simd", "sym", "circle", "latency", "multithr");
    for (size_t i = 0; i < steps; i++) {
        printf("%12zu %12f %12f %12f %12f %12f %12f\n",
               r[i].width, r[i].naive, r[i].simd, r[i].sym,
               r[i].circle, r[i].latency, r[i].multithreading);
    }
}

int testSuite() {
    printf("Starte Performance tests:\n");

    timeMeasurement* time_table = measureAgainstSize(0, 200, 50, 1.0f, 256, false);
    printTable(time_table, 100);

    free(time_table);
    printf("\nTest Suite Fertig\n");
    return 0;
}

// Für neue Testfällen einfach neuen testCase() in testSuite() hinzufügen

// Bei neuen Implementationen/Versionen in testAllVersions() das Schleifenlimit
// erhöhen und die neue Version als if else hinzufügen