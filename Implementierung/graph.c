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

timeMeasurement* plotTableGeneration(const size_t steps, const size_t step_length, TestCase tc) {
    timeMeasurement* results = malloc(sizeof(timeMeasurement) * steps);
    if (results == NULL) {
        fprintf(stderr, "Error allocating memory\n");
        return NULL;
    }

    for (size_t step = 0; step < steps; step++) {
        const size_t length = tc.width + step * step_length;

        const float res = (tc.res == 0.0) ? (4.0f / (float) length) : tc.res;

        results[step].width = length;
        results[step].iterations = tc.n;
        results[step].res = res;
        results[step].step = step;

        TestCase tc_step = {tc.start, length, (ssize_t) length, res, tc.n, tc.color, "plot"};
        results[step].naive = performanceTest(tricorn_V1, tc_step, 1);
        results[step].simd = performanceTest(tricorn_V2, tc_step, 1);
        results[step].circle = performanceTest(tricorn_V3, tc_step, 1);
        results[step].latency = performanceTest(tricorn_V4, tc_step, 1);
        results[step].multithreading = performanceTest(tricorn, tc_step, 1);
        printf("Benchmark Fortschritt: %zu/%zu\n", step + 1, steps);
    }
    return results;
}

void printTable(const timeMeasurement* r, size_t steps) {
    if (r == NULL) {
        printf("Passed Pointer was NULL\n");
        return;
    }
    printf("\n");
    printf("%12s %12s %12s | %13s %13s %13s %13s %13s\n",
           "step", "width", "res", "naive(s)", "simd(s)", "circle(s)", "latency(s)", "multithr(s)");
    for (size_t i = 0; i < steps; i++) {
        printf("%12u %12zu %12f | %13f %13f %13f %13f %13f\n",
               r[i].step, r[i].width, r[i].res, r[i].naive, r[i].simd, r[i].circle, r[i].latency,
               r[i].multithreading);
    }
    printf("\n");
}

int generateCustomPlottableTable(size_t steps, TestCase startTestCase, size_t step_length) {
    timeMeasurement* table = plotTableGeneration(steps, step_length, startTestCase);
    printTable(table, steps);

    free(table);
    return 0;
}


int generatePlottableTable() {
    size_t steps = 10;
    TestCase startTestCase = {-2.0f + -2.0f * I, 50, 50, 0.0f, 256, false, "plot"};
    size_t step_length = 100;

    timeMeasurement* table = plotTableGeneration(steps, step_length, startTestCase);
    printTable(table, steps);

    free(table);
    return 0;
}