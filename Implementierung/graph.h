#ifndef GRA26CAPSPROJECT_T105_GRAPH_H
#define GRA26CAPSPROJECT_T105_GRAPH_H

#include <complex.h>
#include <stdio.h>
#include <stdbool.h>
#include "testSuite.h"


typedef struct {
    double naive;
    double simd;
    double circle;
    double latency;
    double multithreading;
    size_t width;
    unsigned iterations;
    float res;
    int step;
} timeMeasurement;

int generateCustomPlottableTable(size_t steps, TestCase startTestCase, size_t step_length);
int generatePlottableTable();

#endif
