//
// Created by Hendrik on 14.06.2026.
//

#ifndef GRA26CAPSPROJECT_T105_SYMI_H
#define GRA26CAPSPROJECT_T105_SYMI_H
#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>
#include <complex.h>

void symGenImage(float complex start, size_t width, ssize_t height, float res, unsigned n, bool color, unsigned char* img);

#endif //GRA26CAPSPROJECT_T105_SYM_H
