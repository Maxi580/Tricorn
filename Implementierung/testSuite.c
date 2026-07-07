#include <stdio.h>
#include <complex.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#include "bitmap.h"
#include "NaiveImplementation/image.h"
#include "Simd/image.h"
#include "Simd/escapetime.h"
#include "Sym/image.h"
#include "Circle/image.h"
#include "LatencyThroughput/image.h"
#include "testSuite.h"

int testAllVersions(const float complex start, const size_t width, const ssize_t height_signed, const float res,
    const unsigned n, const bool color, unsigned char* img, const size_t benchmarkIterations) {

    printf("Start:\t\t%f + %2fi\n", crealf(start), cimagf(start));
    printf("Breite:\t\t%zu\n", width);
    printf("Höhe:\t\t%zd\n", height_signed);
    printf("Auflösung\t%f\n", res);
    printf("Iterationen n:\t%u\n", n);
    printf("Farbe:\t\t%s\n", color ? "true" : "false");
    // printf("img: %p\n", img);
    printf("Benchmark n:\t%zu\n", benchmarkIterations);

    for (size_t v=0; v < 5; v++) {
        struct timespec time_start; // Zeitmessung (siehe Folien Benchmarking)
        clock_gettime(CLOCK_MONOTONIC, &time_start);

        if (v == 0) {
            for (size_t i=0; i < benchmarkIterations; i++) {
                genImage(start, width, height_signed, res, n, color, img);
            }
        } else if (v == 1) {
            for (size_t i=0; i < benchmarkIterations; i++) {
                genImageSIMD(start, width, height_signed, res, n, color, img);
            }
        } else if (v == 2) {
            for (size_t i=0; i < benchmarkIterations; i++) {
                symGenImage(start, width, height_signed, res, n, color, img);
            }
        } else if (v == 3) {
            for (size_t i=0; i < benchmarkIterations; i++) {
                genImageCircle(start, width, height_signed, res, n, color, img);
            }
        } else if (v == 4) {
            for (size_t i=0; i < benchmarkIterations; i++) {
                genImageLatencyThroughput(start, width, height_signed, res, n, color, img);
            }
        } else {
            printf("Error: Implementation existiert nicht");
            free(img);
            return 1;
        }

        struct timespec time_end;
        clock_gettime(CLOCK_MONOTONIC, &time_end);
        double time = time_end.tv_sec - time_start.tv_sec + 1e-9 * (time_end.tv_nsec - time_start.tv_nsec);
        if (time < 1) { // unter 1 Sekunde getestet (ungültig)
            printf("Error: Gesamtlaufzeit zu kurz. Erhöhe die Iterationen! (%f s < 1 s)\n", time);
            return 1;
        }
        double avg_time = time / benchmarkIterations;
        printf("Version %zu: Dauer pro Iteration: %f s\n", v, avg_time);
    }

    return 0;
}

int testCase(const char *name, const float complex start, const size_t width, const ssize_t height_signed,
               const float res, const unsigned n, const bool color, char *filename,
               const size_t benchmarkIterations) {

    printf("\nTest:\t\t%s\n", name);

    size_t height;
    if (height_signed < 0) {
        height = height_signed * (-1);
    }
    else {
        height = height_signed ;
    }

    size_t row_size = ((width * 3+3)/4)*4;
    size_t pixelArraySize = row_size * height;

    unsigned char *img = malloc(pixelArraySize);
    if (img == NULL) { //Fehler
        printf("Error allocating memory\n");
        return 1;
    }

    int runTest = testAllVersions(start, width, height_signed, res, n, color, img, benchmarkIterations);

    if (runTest == 0) {
        writeBmp(filename, width, height_signed, img);
    }

    free(img);
    return 0;
}

int testSuite() {
    printf("Starte Test Suite:\n");


    testCase(
        "klein",
        0.0 + 0.0 * I,
        40,
        40,
        4.0f/40,
        256,
        false,
        "klein.bmp",
        100000
    );

    testCase(
        "mittel",
        0.0 + 0.0 * I,
        4000,
        4000,
        4.0f/4000,
        256,
        false,
        "mittel.bmp",
        15
    );

    testCase(
        "groß",
        0.0 + 0.0 * I,
        15000,
        15000,
        4.0f/15000,
        256,
        false,
        "groß.bmp",
        5
    );


    printf("\nTest Suite Fertig\n");
    return 0;
}

// Für neue Testfällen einfach neuen testCase() in testSuite() hinzufügen

// Bei neuen Implementationen/Versionen in testAllVersions() das Schleifenlimit
// erhöhen und die neue Version als if else hinzufügen