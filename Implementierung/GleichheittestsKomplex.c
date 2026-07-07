
#include <complex.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "bitmap.h"
#include "NaiveImplementation/image.h"
#include "Simd/image.h"
#include "Simd/escapetime.h"
#include "Sym/image.h"
#include "Circle/image.h"
#include "LatencyThroughput/image.h"
#include "testSuite.h"
#include "NaiveImplementation/escapetime.h"
#include <stdio.h>
#include <complex.h>
#include <immintrin.h>

#include "Circle/escapetime.h"
#include "LatencyThroughput/escapetime.h"
#include "Sym/escapetime.h"
#include "Simd/escapetime.h"
#include "Sym/escapetime.h"


bool compareCorrectnessTest(float complex start, size_t width, ssize_t heightsigned, float res, unsigned n, bool color) {
    printf("starte test\n");
    size_t height;
    if (heightsigned < 0) {
        height = (size_t)(-heightsigned);
    }
    else {
        height = (size_t)heightsigned ;
    }

    size_t row_size = ((width * 3+3)/4)*4;
    size_t pixelArraySize = row_size * height;

    unsigned char *v0 = malloc(pixelArraySize);
    unsigned char *v1 = malloc(pixelArraySize);
    unsigned char *v2 = malloc(pixelArraySize);
    unsigned char *v3 = malloc(pixelArraySize);
    unsigned char *v4 = malloc(pixelArraySize);

    if (v0 == NULL || v1 == NULL || v2 == NULL || v3 == NULL || v4 == NULL) {
        printf("malloc() returned NULL\n");
        free(v0);
        free(v1);
        free(v2);
        free(v3);
        free(v4);
        return false;
    }

    genImage(start, width, heightsigned, res, n, color, v0);
    genImageSIMD(start, width, heightsigned, res, n, color, v1);
    symGenImage(start, width, heightsigned, res, n, color, v2);
    genImageCircle(start, width, heightsigned, res, n, color, v3);
    genImageLatencyThroughput(start, width, heightsigned, res, n, color, v4);

    bool AllEqual = true;

    if (memcmp(v0, v1, pixelArraySize) == 0) {
        printf("v0 und v1 identisch\n");

    }
    else {
        for (size_t i = 0; i < pixelArraySize; i++) {
            if (v0[i] != v1[i]) {
                printf("Erster Unterschied v0/v1 bei Byte %zu: %u vs %u\n", i, v0[i], v1[i]);
                break;
            }
        }
        printf("v0 and v1 nicht identisch\n");
        AllEqual = false;
    }

    if (memcmp(v0, v2, pixelArraySize) == 0) {
        printf("v0 and v2 identisch\n");
    }
    else {
        for (size_t i = 0; i < pixelArraySize; i++) {
            if (v0[i] != v2[i]) {
                printf("Erster Unterschied v0/v2 bei Byte %zu: %u vs %u\n", i, v0[i], v2[i]);
                break;
            }
        }
        printf("v0 and v2 nicht identisch\n");
        AllEqual = false;
    }
    if (memcmp(v0, v3, pixelArraySize) == 0) {
        printf("v0 and v3 identisch\n");
    }
    else {
        for (size_t i = 0; i < pixelArraySize; i++) {
            if (v0[i] != v3[i]) {
                printf("Erster Unterschied v0/v3 bei Byte %zu: %u vs %u\n", i, v0[i], v3[i]);
                break;
            }
        }
        printf("v0 and v3 nicht identisch\n");
        AllEqual = false;
    }
    if (memcmp(v0, v4, pixelArraySize) == 0) {
        printf("v0 and v4 identisch\n");
    }
    else {
        for (size_t i = 0; i < pixelArraySize; i++) {
            if (v0[i] != v4[i]) {
                printf("Erster Unterschied v0/4 bei Byte %zu: %u vs %u\n", i, v0[i], v4[i]);
                break;
            }
        }
        printf("v0 and v4 nicht identisch\n");
        AllEqual = false;
    }
    free(v0);
    free(v1);
    free(v2);
    free(v3);
    free(v4);
    return AllEqual;
}
void debug(void) {
    unsigned n = 100;
    float complex c = -1.32f - 0.035f * I;
    printf("c = %.9f %+.9fi\n", crealf(c), cimagf(c));

    printf("naive: %u\n", escapeTime(n, c));
    __m128i result =  simdEscapeTime(n, c, c, c, c);
    unsigned values[4];
    _mm_storeu_si128((__m128i*)values, result);
    printf("simd: %u\n", values[0]);

    result = symEscapeTime(n, c, c, c, c);
    _mm_storeu_si128((__m128i*)values, result);
    printf("sym: %u\n", values[0]);
    printf("\n");
    printf("\n");

    printf("upper: %u\n", escapeTime(100, -1.32f + 0.035f * I));
    printf("lower: %u\n", escapeTime(100, -1.32f - 0.035f * I));

}

bool BitmapVergleichTests() {
    printf("BitmapVergleichTests:\n");
    struct TestCase {
        float complex start;
        size_t width;
        ssize_t height;
        float res;
        unsigned n;
        bool color;
        const char *name;
    };

    struct TestCase testCases[] = {
        {-2.0 +1.5f * I, 800, -600, 0.005f, 100 ,false, "topDownGray"},
        {-2.0 -1.5f * I, 800, +600, 0.005f, 100 ,false, "BottomUpGray"},
        {-2.0 +1.5f * I, 800, -600, 0.005f, 100 ,false, "TopDownGray"},
        {-2.0 -1.5f * I, 800, +600, 0.005f, 100 ,false, "BottomUpGray"},

        {-0.5 +2.0f * I, 200, -200, 0.01f, 50 ,false, "randomGrayscale"},
        {-2.0 +1.5f * I, 800, 600, 0.005f, 100 ,false, "OutOfSCope"},

        {-2.0 +2.0f * I, 1, -1, 0.01f, 10 ,false, "1x1 topDonw"},
        {-2.0 -2.0f * I, 2, 2, 0.01f, 10 ,false, "2x2 BottomUp"},

        {-2.0 +2.0f * I, 3, -3, 0.01f, 10 ,false, "3x3 topDown"},
        {-2.0 +2.0f * I, 3, -300, 0.01f, 10 ,false, "3x300 "},

        {-0.5 +0.5 * I, 300, -200, 0.001f, 1500 ,false, "high iterations "},



    };
    size_t testCasesCount = sizeof(testCases)/sizeof(testCases[0]);
    size_t passed = 0;
    for (size_t i = 0; i < testCasesCount; i++) {
        printf("\nTest %zu/%zu: %s\n", i+1, testCasesCount, testCases[i].name);

        bool ok = compareCorrectnessTest(testCases[i].start, testCases[i].width, testCases[i].height, testCases[i].res, testCases[i].n, testCases[i].color);
        if (!ok) {
            printf("Fehlgeschlagen: %s\n", testCases[i].name);
        }
        if (ok) {
            printf("Bestanden: %s\n", testCases[i].name);
            passed++;
        }
    }
    printf("\nErgebnis: %zu/%zu Tests bestanden. \n", passed, testCasesCount);

    printf("\n");
    debug();
    printf("\n");
    if (passed != testCasesCount) {
        return false;
    }
    return true;

}
