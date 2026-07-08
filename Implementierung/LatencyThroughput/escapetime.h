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

typedef struct {
	__m128i a;
	__m128i b;
	__m128i c;
	__m128i d;
} fourIntegerLanes;

typedef struct {
	__m128 a;
	__m128 b;
	__m128 c;
	__m128 d;
} fourFloatLanes;


twoIntegerLanes escapeTimeLatencyThroughputTwo(const unsigned n, const float complex a1, const float complex a2, const float complex a3,
	const float complex a4, const float complex b1, const float complex b2, const float complex b3,
	const float complex b4);

fourIntegerLanes escapeTimeLatencyThroughputFour(const unsigned n, const float complex a1, const float complex a2, const float complex a3,
	const float complex a4, const float complex b1, const float complex b2, const float complex b3,
	const float complex b4, const float complex c1, const float complex c2, const float complex c3,
	const float complex c4, const float complex d1, const float complex d2, const float complex d3,
	const float complex d4);

#endif
