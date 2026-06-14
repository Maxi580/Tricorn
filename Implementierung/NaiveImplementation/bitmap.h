//
// Created by Hendrik on 14.06.2026.
//

#ifndef GRA26CAPSPROJECT_T105_BASICTESTIMPLEMENTATION_H
#define GRA26CAPSPROJECT_T105_BASICTESTIMPLEMENTATION_H
#include <stddef.h>
#include <sys/types.h>

void genImage(size_t width, ssize_t height, unsigned char *img);
void writeBmp(char *filename, size_t width, ssize_t heightsigned, unsigned char *img);


#endif //GRA26CAPSPROJECT_T105_BASICTESTIMPLEMENTATION_H
