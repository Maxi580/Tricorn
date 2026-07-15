#ifndef BITMAP_H
#define BITMAP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#define BMP_MAX_FILE_SIZE UINT32_MAX
#define BMP_MAX_RANGE INT32_MAX
#define BMP_MIN_RANGE INT32_MIN
#define BMP_HEADER_SIZE 54
#define COLOR_PALETTE_SIZE 0
#define GREY_PALETTE_SIZE 1024

int writeBmp(char *filename, ssize_t width, ssize_t height_signed, unsigned char *img, bool color);

#endif
