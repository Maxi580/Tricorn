#ifndef GRA26CAPSPROJECT_T105_COLORMAP_H
#define GRA26CAPSPROJECT_T105_COLORMAP_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} RGB_color;

static const RGB_color color_pallete[4] = {
    // Red, Green, Blue
    {0, 0, 128},
    {0, 0, 255},
    {0, 100, 255},
    {0, 255, 255},
};
static const unsigned color_pallete_size = sizeof(color_pallete) / sizeof(color_pallete[0]);

static const RGB_color black = {0, 0, 0};

static RGB_color getColor(unsigned limit, unsigned iterations, bool color) {
    if(iterations == limit) {
        return black;
    }
    if(color) {
        return color_pallete[iterations % color_pallete_size];
    }
    unsigned grey_color = 255 - 4 * (iterations % 64);
    return (RGB_color) {grey_color, grey_color, grey_color};
}

static inline size_t abs_size(ssize_t v) {
    // Cleanly extracts the absolute value of a ssize_t into a size_t
    // size_t 0 - 2^64 - 1  /   ssize_t  -2^63 - 2^63 - 1
    // size_t cast to negatve values adds 2^64, then you need to "invert" it and with - you do 2^64 - v
    return v < 0 ? -(size_t) v : (size_t) v;
}

#endif //GRA26CAPSPROJECT_T105_COLORMAP_H
