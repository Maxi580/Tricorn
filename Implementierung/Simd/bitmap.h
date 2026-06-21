//
// Created by Hendrik on 14.06.2026.
//

#ifndef GRA26CAPSPROJECT_T105_SIMD_H
#define GRA26CAPSPROJECT_T105_SIMD_H
#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

void genImageSIMD(float complex start, size_t width, ssize_t height, float res, unsigned n, bool color, unsigned char* img);
void writeBmpSIMD(char *filename, size_t width, ssize_t heightsigned, unsigned char *img);


#endif //GRA26CAPSPROJECT_T105_BASICTESTIMPLEMENTATION_H
