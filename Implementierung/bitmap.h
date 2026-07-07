#ifndef BITMAP_H
#define BITMAP_H

#include <stddef.h>
#include <sys/types.h>

void writeBmp(char *filename, size_t width, ssize_t heightsigned, unsigned char *img);

#endif
