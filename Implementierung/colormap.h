#ifndef GRA26CAPSPROJECT_T105_COLORMAP_H
#define GRA26CAPSPROJECT_T105_COLORMAP_H

#include <stdint.h>
#include <stdbool.h>

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

static inline RGB_color getColor(unsigned limit, unsigned iterations, bool color) {
    if(iterations == limit) {
        return black;
    }
    if(color) {
        return color_pallete[iterations % color_pallete_size];
    }
    unsigned grey_color = 255 - 4 * (iterations % 64);
    return (RGB_color) {grey_color, grey_color, grey_color};
}

#endif //GRA26CAPSPROJECT_T105_COLORMAP_H
