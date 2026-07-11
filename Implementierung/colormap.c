#include <complex.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "colormap.h"

const RGB_color color_pallete[4] = {
    // We call modulo with the size of this pallete, quite a few times, it would be nicer if this is a multiple of 2 ig
    // Red, Green, Blue
    {0, 0, 128},
    {0, 0, 255},
    {0, 100, 255},
    {0, 255, 255},
};
const unsigned color_pallete_size = sizeof(color_pallete) / sizeof(color_pallete[0]);

RGB_color* createColorMap(unsigned limit, bool color) {
    RGB_color* mapping = malloc((limit + 1) * sizeof(RGB_color));
    if (mapping == NULL) {
        return NULL;
    }

    for (unsigned i = 0; i < limit; i++) {
        if (color) {
            unsigned color_index = i % color_pallete_size;
            mapping[i] = color_pallete[color_index];
        } else {
            unsigned grey_color = 255 - 4 * (i % 64 );
            mapping[i] = (RGB_color) {grey_color, grey_color, grey_color};
        }
    }
    mapping[limit] = (RGB_color) {0, 0, 0};
    return mapping;
}