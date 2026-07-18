#ifndef GRA26CAPSPROJECT_T105_GRAPH_H
#define GRA26CAPSPROJECT_T105_GRAPH_H

#include <complex.h>
#include <stdio.h>
#include <stdbool.h>

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

int generatePlottableTable();

#endif
