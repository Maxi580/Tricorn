#ifndef GRA26CAPSPROJECT_T105_TESTSUITE_H
#define GRA26CAPSPROJECT_T105_TESTSUITE_H

#include <complex.h>
#include <stdio.h>
#include <stdbool.h>

#define MIN_TEST_DURATION 1.0

typedef struct {
    float complex start;
    size_t width;
    ssize_t height_signed;
    float res;
    unsigned n;
    bool color;
    const char *name;
} TestCase;

bool testSuite();
double performanceTest(void (*fn)(float complex, size_t, ssize_t, float,  unsigned, bool, unsigned char *),
                        TestCase tc, size_t iterations);

#endif //GRA26CAPSPROJECT_T105_TESTSUITE_H
