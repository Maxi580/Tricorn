#ifndef GRA26CAPSPROJECT_T105_UTILS_H
#define GRA26CAPSPROJECT_T105_UTILS_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static inline size_t abs_size(ssize_t v) {
    // Cleanly extracts the absolute value of a ssize_t into a size_t
    // size_t 0 - 2^64 - 1  /   ssize_t  -2^63 - 2^63 - 1
    // size_t cast to negatve values adds 2^64, then you need to "invert" it and with - you do 2^64 - v
    return v < 0 ? -(size_t) v : (size_t) v;
}

#endif //GRA26CAPSPROJECT_T105_UTILS_H
