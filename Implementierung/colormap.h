#ifndef GRA26CAPSPROJECT_T105_COLORMAP_H
#define GRA26CAPSPROJECT_T105_COLORMAP_H

#include <stdint.h>
#include <stdbool.h>

#define rgbSize 3

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} RGB_color;

RGB_color* createColorMap(unsigned limit, bool color);

#endif //GRA26CAPSPROJECT_T105_COLORMAP_H
