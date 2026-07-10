#ifndef GRA26CAPSPROJECT_T105_MT_H
#define GRA26CAPSPROJECT_T105_MT_H
#include <stdbool.h>
#include <complex.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>


void tricornMultiThreading(float complex start, size_t width, ssize_t height, float res, unsigned n, bool color, unsigned char* img);

#endif
