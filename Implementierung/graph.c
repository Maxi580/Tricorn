#include <complex.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "NaiveImplementation/tricorn_V1.h"
#include "Simd/tricorn_V2.h"
#include "Sym/tricorn_V3.h"
#include "Circle/tricorn_V4.h"
#include "LatencyThroughput/tricorn_V5.h"
#include "MultiThreading/tricorn.h"
#include "testSuite.h"
#include "bitmap.h"
#include "graph.h"
#include "testSuite.h"

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
        results[step].iterations = n;
        results[step].res = res;

        TestCase tc_step = {tc.start, length, (ssize_t) length, res, n, tc.color, "plot"};
        results[step].naive = performanceTest(tricorn_V1, tc_step, 1);
        results[step].simd = performanceTest(tricorn_V2, tc_step, 1);
        results[step].sym = performanceTest(tricorn_V3, tc_step, 1);
        results[step].circle = performanceTest(tricorn_V4, tc_step, 1);
        results[step].latency = performanceTest(tricorn_V5, tc_step, 1);
        results[step].multithreading = performanceTest(tricorn, tc_step, 1);
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

int generatePlottableTable() {
    size_t steps = 4;

    TestCase tc = {-2.0f * -2.0f, 200, 200, 0.0f, 256, false, "plot"};

    timeMeasurement* table = plotTableGeneration(steps, 50, 0, tc);
    printTable(table, steps);

    free(table);
    return 0;
}