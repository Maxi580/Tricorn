
#ifndef GRA26CAPSPROJECT_T105_GLEICHHEITTESTSKOMLEX_H
#define GRA26CAPSPROJECT_T105_GLEICHHEITTESTSKOMLEX_H

#include <complex.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "bitmap.h"
#include "NaiveImplementation/image.h"
#include "Simd/image.h"
#include "Simd/escapetime.h"
#include "Sym/image.h"
#include "Circle/image.h"
#include "LatencyThroughput/image.h"
#include "testSuite.h"
bool compareCorrectnessTest(float complex start, size_t width, ssize_t height_signed, float res, unsigned n, bool color);
bool BitmapVergleichTests();
#endif //GRA26CAPSPROJECT_T105_GLEICHHEITTESTSKOMLEX_H
