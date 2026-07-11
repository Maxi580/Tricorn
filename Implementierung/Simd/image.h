#ifndef GRA26CAPSPROJECT_T105_SIMD_IMAGE_H
#define GRA26CAPSPROJECT_T105_SIMD_IMAGE_H
#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>
#include <complex.h>

void tricornSimd(float complex start, size_t width, ssize_t height, float res, unsigned n, bool color, unsigned char* img);

#endif //GRA26CAPSPROJECT_T105_SIMD_H
