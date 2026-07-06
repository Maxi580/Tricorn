//
// Created by Hendrik on 14.06.2026.
//

#ifndef GRA26CAPSPROJECT_T105_LT_H
#define GRA26CAPSPROJECT_T105_LT_H

#include <complex.h>
#include <immintrin.h>

typedef struct {
    __m128i a;
    __m128i b;
} twoIntegerLanes;

typedef struct {
    __m128 a;
    __m128 b;
} twoFloatLanes;


twoIntegerLanes escapeTimeLatencyThroughput(const unsigned n, const float complex a1, const float complex a2, const float complex a3,
    const float complex a4, const float complex b1, const float complex b2, const float complex b3,
    const float complex b4);

#endif //GRA26CAPSPROJECT_T105_LT_H
