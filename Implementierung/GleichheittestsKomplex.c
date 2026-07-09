
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
#include "MultiThreading/image.h"

bool compareImages(unsigned char *v0, unsigned char *v1, size_t pixelArraySize) {
    for (size_t i = 0; i < pixelArraySize; i++) {
        if (v0[i] != v1[i]) {
            printf("Bytes v0: %d / v1: %d", v0[i], v1[i]);
            return false;
        }
    }
    return true;
}

bool compareCorrectnessTest(float complex start, size_t width, ssize_t height_signed, float res, unsigned n, bool color) {
    size_t height;
    bool all_equal = true;
    if (height_signed < 0) {
        height = (size_t)(-height_signed);
    }
    else {
        height = (size_t)height_signed ;
    }

    size_t row_size = ((width * 3+3)/4)*4;
    size_t pixelArraySize = row_size * height;

    unsigned char *v0 = malloc(pixelArraySize);
    unsigned char *v1 = malloc(pixelArraySize);

    if (v0 == NULL || v1 == NULL) {
        printf("malloc() returned NULL\n");
        free(v0);
        free(v1);
        return false;
    }

    genImage(start, width, height_signed, res, n, color, v0);
    genImageSIMD(start, width, height_signed, res, n, color, v1);
    if (!compareImages(v0, v1, pixelArraySize)) {
        printf("SIMD Error on randomized Tests\n");
        all_equal = false;
    }
    symGenImage(start, width, height_signed, res, n, color, v1);
    if (!compareImages(v0, v1, pixelArraySize)) {
        printf("SIMD Version Error on randomized Tests\n");
        all_equal = false;
    }
    genImageCircle(start, width, height_signed, res, n, color, v1);
    if (!compareImages(v0, v1, pixelArraySize)) {
        printf("Circle Version Error on randomized Tests\n");
        all_equal = false;
    }
    genImageLatencyThroughput(start, width, height_signed, res, n, color, v1);
    if (!compareImages(v0, v1, pixelArraySize)) {
        printf("LatencyThroughput Version Error on randomized Tests\n");
        all_equal = false;
    }
    genImageMultiThreading(start, width, height_signed, res, n, color, v1);
    if (!compareImages(v0, v1, pixelArraySize)) {
        printf("Multithreading Version Error on randomized Tests\n");
        all_equal = false;
    }

    free(v0);
    free(v1);
    return all_equal;
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

bool randomizedTests(size_t width, ssize_t height_signed, int trials){
    printf("Starte randomisierte/ bruteforce tests\n");
    bool all_true = true;

    float max = 2.0f;
    float min = -2.0f;
    unsigned n = 256;
    bool color = false;
    for (int iterations = 0; iterations < trials; iterations++) {
        float start_real = min + (max - min) * ((float) rand()/ (float) RAND_MAX);
        float start_imag = min + (max - min) * ((float) rand()/ (float) RAND_MAX);
        float complex start = start_real + start_imag * I;
        float res = 100.0f * ((float) rand()/ (float) RAND_MAX);

        all_true = compareCorrectnessTest(start, width, height_signed, res, n, color) ? all_true : false;
    }

    return all_true;
}

bool symmetryTest(size_t width, ssize_t height_signed, int trials) {
    // Randomized tests, that fix the center around an even y so that symmetry would be applied
    // The Imaginary Value at a y is heavily dependet on res, with weird/small resolutions, weird stuff happens
    // like mirrored coordinates not having the exact same imag value.
    printf("Starte Symmetrie tests\n");
    bool all_true = true;
    size_t height = height_signed < 0 ? -height_signed : height_signed;
    float factor = height_signed < 0 ? -1.0f : 1.0f;

    unsigned n = 100;
    bool color = false;
    srand(3141592653);
    for (int i = 0; i < trials;  trials++){
        int center = height / 2 + rand() % 10; // Center must be at an even y so symmetry should trigger
        // Random small res, so that the imags of two mirrored ys are not cleanly the same
        float res = 0.002f + 0.011f * ((float) rand()/(float) RAND_MAX);
        float start_real = -2.0f + 4.0f * ((float) rand()/ (float) RAND_MAX);
        float start_imag = -(center * factor * res); // set imag so that the center is at center
        float complex start = start_real + start_imag * I;

        all_true = compareCorrectnessTest(start, width, height_signed, res, n, color) ? all_true : false;
    }
    return all_true;
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

    bool result = randomizedTests(300, 300, 100);
    if (!result) {
        printf("Randomized tests failed\n");
    } else {
        printf("Randomized tests passed\n");
    }

    result = symmetryTest(30, 300, 100);
    if (!result) {
        printf("Symmetry tests failed\n");
    } else {
        printf("Symmetry tests passed\n");
    }


    printf("\n");
    debug();
    printf("\n");
    if (passed != testCasesCount) {
        return false;
    }
    return true;

}
