#include <complex.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "NaiveImplementation/tricorn_V1.h"
#include "Simd/tricorn_V2.h"
#include "Circle/tricorn_V3.h"
#include "LatencyThroughput/tricorn_V4.h"
#include "MultiThreading/tricorn.h"
#include "testSuite.h"
#include "bitmap.h"
#include "graph.h"
#include "testSuite.h"
#include "utils.h"
#include <time.h>

timeMeasurement* plotTableGeneration(const size_t steps, const size_t step_length, const size_t step_n, TestCase tc) {
    timeMeasurement* results = malloc(sizeof(timeMeasurement) * steps);
    if (results == NULL) {
        printf("malloc() returned NULL\n");
        return NULL;
    }

    for (size_t step = 0; step < steps; step++) {
        const size_t length = tc.width + step * step_length;
        const size_t n = tc.n + step * step_n;

        const float res = (tc.res == 0.0) ? (4.0f / (float) length) : tc.res;

        results[step].width = length;
        results[step].iterations = step;
        results[step].res = res;

        TestCase tc_step = {tc.start, length, (ssize_t) length, res, n, tc.color, "plot"};
        results[step].naive = performanceTest(tricorn_V1, tc_step, 1);
        results[step].simd = performanceTest(tricorn_V2, tc_step, 1);
        results[step].circle = performanceTest(tricorn_V3, tc_step, 1);
        results[step].latency = performanceTest(tricorn_V4, tc_step, 1);
        results[step].multithreading = performanceTest(tricorn, tc_step, 1);
    }
    return results;
}

/*double measureThreads(TestCase tc, size_t iterations, size_t thread_amount) {
    int bytesPerPixel = tc.color ? 3 : 1;
    size_t height = abs_size(tc.height_signed);
    size_t row_size = ((tc.width * bytesPerPixel+3)/4)*4;
    size_t pixelArraySize = row_size * height;
    unsigned char *img = malloc(pixelArraySize);
    if (img == NULL) {
        fprintf(stderr, "malloc() returned NULL\n");
        return -1.0f;
    }

    struct timespec time_start, time_end;
    double time = 0;
    bool test_is_valid = false;

    while (!test_is_valid) {
        clock_gettime(CLOCK_MONOTONIC, &time_start);
        for (size_t i = 0; i < iterations; i++) {
            tricornThreadPlot(tc.start, tc.width, tc.height_signed, tc.res, tc.n, tc.color, img, thread_amount);
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

timeMeasurement* plotTableThreads(const size_t steps, TestCase tc) {
    timeMeasurement* results = malloc(sizeof(timeMeasurement) * steps);
    if (results == NULL) {
        printf("malloc() returned NULL\n");
        return NULL;
    }
    for (size_t step = 0; step < steps; step++) {
        results[step].multithreading = measureThreads(tc, 10, step + 1);
    }
    return results;
}*/

void printTable(const timeMeasurement* r, size_t steps) {
    if (r == NULL) {
        printf("Passed Pointer was NULL");
        return;
    }

    printf("%12s %12s %12s %12s %12s %12s %12s %12s\n",
           "width", "iter", "res", "naive", "simd", "circle", "latency", "multithr");
    for (size_t i = 0; i < steps; i++) {
        printf("%12zu %12u %12f %12f %12f %12f %12f %12f\n",
               r[i].width, r[i].iterations, r[i].res, r[i].naive, r[i].simd, r[i].circle, r[i].latency,
               r[i].multithreading);
    }
}

int generatePlottableTable() {
    size_t steps = 200;

    TestCase tc = {-2.0f + -2.0f * I, 50, 50, 0.0f, 256, false, "plot"};

    timeMeasurement* table = plotTableGeneration(steps, 50, 0, tc);
    printTable(table, steps);

    free(table);
    return 0;
}