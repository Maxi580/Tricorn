#include <complex.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "bitmap.h"
#include <time.h>

#include "NaiveImplementation/tricorn_V1.h"
#include "Simd/tricorn_V2.h"
#include "Circle/tricorn_V3.h"
#include "LatencyThroughput/tricorn_V4.h"
#include "MultiThreading/tricorn.h"
#include "utils.h"
#include "testSuite.h"
#include "graph.h"


double performanceTest(void (*fn)(float complex, size_t, ssize_t, float,  unsigned, bool, unsigned char *),
                        TestCase tc, size_t iterations) {
    /* Misst die Zeit eines übergebenen Testcases so oft wie in iterations angegeben bzw erhöht diese bis der
     * Test valide ist.*/
    // Speicher allozieren
    int bytesPerPixel = tc.color ? 3 : 1;
    size_t height = abs_size(tc.height_signed);
    size_t row_size = ((tc.width * bytesPerPixel+3)/4)*4;
    size_t pixelArraySize = row_size * height;
    unsigned char *img = malloc(pixelArraySize);
    if (img == NULL) {
        fprintf(stderr, "Error allocating memory\n");
        exit(EXIT_FAILURE);
    }

    struct timespec time_start, time_end;
    double min_time = 0.0;
    double time = 0;

    // Der Test muss mindestens 3 mal wiederholt werden (Wir nehmen minimum, da Störungen, den Test langsamer machen)
    int repetitions = 3;
    for (int rep = 0; rep < repetitions; rep++) {
        bool test_is_valid = false;
        // Ein Test ist erst gültig, wenn er länger als 1sek ist
        while (!test_is_valid) {
            clock_gettime(CLOCK_MONOTONIC, &time_start);
            for (size_t i = 0; i < iterations; i++) {
                fn(tc.start, tc.width, tc.height_signed, tc.res, tc.n, tc.color, img);
            }
            clock_gettime(CLOCK_MONOTONIC, &time_end);

            time = (double)(time_end.tv_sec - time_start.tv_sec) + 1e-9 * (time_end.tv_nsec - time_start.tv_nsec);
            if (time < MIN_TEST_DURATION) { // unter 1 Sekunde getestet (ungültig) → erhöhe iterationen
                size_t increased_iterations = (time > 0.0) ?
                                (size_t) ((double) iterations * (1.3 * MIN_TEST_DURATION/time)) : 0;
                iterations = (increased_iterations > iterations) ? increased_iterations : (iterations + 1);
            } else {
                double new_time = time / (double) iterations;
                if (rep == 0) {
                    min_time = new_time;
                } else {
                    min_time = new_time < min_time ? new_time : min_time;
                }

                test_is_valid = true;
            }
        }
    }
    free(img);
    return min_time;
}

void printDebug(unsigned char *base, unsigned char *test, size_t pixel_array_size) {
    for (size_t i = 0; i < pixel_array_size; i++) {
        if (base[i] != test[i]) {
            printf("Erster Unterschied bei Byte %zu: %u vs %u\n", i, base[i], test[i]);
            break;
        }
    }
}

bool correctnessTest(TestCase tc, bool print_debug) {
    // Vergleicht die Ausgaben aller optimierten Implementierungen mit der naiven Implementierung
    bool all_passed = true;

    // Speicher allozieren
    int bytesPerPixel = tc.color ? 3 : 1;
    size_t height = abs_size(tc.height_signed);
    size_t row_size = ((tc.width * bytesPerPixel+3)/4)*4;
    size_t pixel_array_size = row_size * height;
    unsigned char *v0 = malloc(pixel_array_size);
    unsigned char *v1 = malloc(pixel_array_size);
    if (v0 == NULL || v1 == NULL) {
        fprintf(stderr, "Error allocating memory\n");
        free(v0);
        free(v1);
        exit(EXIT_FAILURE);
    }
    // Naive Implementierung
    tricorn_V1(tc.start, tc.width, tc.height_signed, tc.res, tc.n, tc.color, v0);

    tricorn_V2(tc.start, tc.width, tc.height_signed, tc.res, tc.n, tc.color, v1);
    if (memcmp(v0, v1, pixel_array_size)) {
        if (print_debug) {
            printf("SIMD Error\n");
            printDebug(v0, v1, pixel_array_size);
        }
        all_passed = false;
    }
    // v1 mit auffälligen Werten füllen, damit nicht beschriebende Bereiche bemerkt werden
    memset(v1, 0xAA, pixel_array_size);

    tricorn_V3(tc.start, tc.width, tc.height_signed, tc.res, tc.n, tc.color, v1);
    if (memcmp(v0, v1, pixel_array_size)) {
        if (print_debug) {
            printf("Circle Version Error\n");
            printDebug(v0, v1, pixel_array_size);
        }
        all_passed = false;
    }
    memset(v1, 0xAA, pixel_array_size);

    tricorn_V4(tc.start, tc.width, tc.height_signed, tc.res, tc.n, tc.color, v1);
    if (memcmp(v0, v1, pixel_array_size)) {
        if (print_debug) {
            printf("Latency Throughput Version Error\n");
            printDebug(v0, v1, pixel_array_size);
        }
        all_passed = false;
    }
    memset(v1, 0xAA, pixel_array_size);

    tricorn(tc.start, tc.width, tc.height_signed, tc.res, tc.n, tc.color, v1);
    if (memcmp(v0, v1, pixel_array_size) != 0) {
        if (print_debug) {
            printf("Multithreading Version Error\n");
            printDebug(v0, v1, pixel_array_size);
        }
        all_passed = false;
    }

    free(v0);
    free(v1);
    return all_passed;
}

bool individualTests(bool print_debug) {
    bool all_passed = true;

    TestCase test_cases[] = {
        // Base Cases
        {-2.0 + 2.0f * I, 800, -600, 0.005f, 256, false, "topDown Grau"},
        {-2.0 + 2.0f * I, 800, -600, 0.005f, 256, true, "topDown Bunt"},
        {-2.0 - 2.0f * I, 800, +600, 0.005f, 256, false, "bottomUp Grau"},
        {-2.0 - 2.0f * I, 800, +600, 0.005f, 256, true, "bottomUp Bunt"},
        {0.0 - 0.0f * I, 800, +800, 0.005f, 256, true, "von Mitte nach rechts oben"},
        {0.0 - 0.0f * I, 800, -800, 0.005f, 256, true, "von Mitte nach rechts unten"},
        {-2.0 - 0.0f * I, 800, 800, 0.005f, 256, true, "oberhalb der X-Achse"},
        {-2.0 - 0.0f * I, 800, -800, 0.005f, 256, true, "unterhalb der X-Achse"},

        // Komplett weiß (alles sofort divergent)
        {2.0 + 2.0f * I, 400, 400, 0.005f, 256, true, "Bild außerhalb des Fraktals"},

        // Sehr kleine Bilder
        {-2.0 + 1.5f * I, 20, -20, 0.005f, 100, false, "20x20 topDown"},
        {-2.0 + 2.0f * I, 5, -5, 0.1f, 10, false, "5x5 topDown"},
        {-2.0 - 2.0f * I, 10, 10, 0.00001f, 10, false, "10x10 bottomUp"},
        {-2.0 - 2.0f * I, 1, 1, 0.05f, 10, false, "1x1 bottomUp"},
        {-2.0 + 2.0f * I, 7, -7, 0.001f, 10, false, "7x7 topDown"},
        {-2.0 + 2.0f * I, 13, -13, 0.01f, 10, false, "3x3 topDown"},

        // Eine Dimension sehr klein
        {-2.0 + 2.0f * I, 3, -300, 0.01f, 10, false, "3x300 topDown"},
        {-2.0 - 2.0f * I, 3, 300, 0.01f, 10, false, "3x300 bottomUp"},
        {-2.0 + 2.0f * I, 300, -3, 0.01f, 10, false, "300x3 topDown"},
        {-2.0 - 2.0f * I, 300, 3, 0.01f, 10, false, "3x300 bottomUp"},

         // Iterations Sonderfälle
        {-2.0 + 2.0f * I, 800, -800, 0.005f, 0, true, "0 Iterations"},
        {-2.0 + 2.0f * I, 800, -800, 0.005f, 1, true, "1 Iterations"},
        {-2.0 + 2.0f * I, 800, -800, 0.005f, 2, true, "2 Iterations"},
        {-2.0 + 2.0f * I, 800, -800, 0.005f, 3, true, "3 Iterations"},
        {-0.5 + 0.5 * I, 300, -200, 0.001f, 1500, false, "1500 Iterations"},

        // Stark rein und rausgezoomed
        {-2.0 -2.0 * I, 300, 200, 0.000001f, 10, false, "zoomed in"},
        {-2.0 -2.0 * I, 300, 200, 10.0f, 10, false, "zoomed out"},

           // Simd macht immer 4er Blöcke, wir testen ob der Rest auch korrekt behandelt wird
        {-2.0 -2.0 * I, 401, 200, 0.005f, 10, false, "Breite mod 4 = 1"},
        {-2.0 -2.0 * I, 402, 200, 0.005f, 10, false, "Breite mod 4 = 2"},
        {-2.0 -2.0 * I, 403, 200, 0.005f, 10, false, "Breite mod 4 = 3"},

           // Latency Throughput macht 8er Blöcke, gleiche Idee wir vorhin, den Rest überprüfen.
        {-2.0 -2.0 * I, 404, 200, 0.005f, 10, false, "Breite mod 8 = 4"},
        {-2.0 -2.0 * I, 405, 200, 0.005f, 10, false, "Breite mod 8 = 5"},
        {-2.0 -2.0 * I, 406, 200, 0.005f, 10, false, "Breite mod 8 = 6"},
        {-2.0 -2.0 * I, 407, 200, 0.005f, 10, false, "Breite mod 8 = 7"},

           // Bei Multithreading, gibt es eine grenze ab der Threading eingesetzt wird (Thread Overhead)
        {-2.0 + -2.0f * I, 239, 239, 0.005f, 100, false, "Multithreading Size < 57.600"},
        {-2.0 + -2.0f * I, 240, 240, 0.005f, 100, false, "Multithreading Size > 57.600"},
    };

    size_t test_case_count = sizeof(test_cases) / sizeof(test_cases[0]);
    size_t passed = 0;

    for (size_t i = 0; i < test_case_count; i++) {
        printf("Test %zu/%zu: %s\n", i+1, test_case_count, test_cases[i].name);
        printf("Start:\t\t%f + %2fi\n", crealf(test_cases[i].start), cimagf(test_cases[i].start));
        printf("Breite:\t\t%zu\n", test_cases[i].width);
        printf("Höhe:\t\t%zd\n", test_cases[i].height_signed);

        printf("Auflösung\t%f\n", test_cases[i].res);
        printf("Iterationen n:\t%u\n", test_cases[i].n);
        printf("Farbe:\t\t%s\n", test_cases[i].color ? "true" : "false");
        bool correct = correctnessTest(test_cases[i], print_debug);

        if (correct) {
            printf("BESTANDEN \n\n");
            passed++;
        } else {
            printf("UNGÜLTIG: Image stimmt nicht mit Naiven Implementation überein\n\n");
            all_passed = false;
        }
    }
    printf("Manuelle Tests %zu/%zu bestanden.\n\n", passed, test_case_count);
    return all_passed;
}

bool randomizedTest(int trials, bool debug){
    // Führt viele randomisierte tests aus, variiert startwerte und resolution
    printf("Starte randomisierte Tests\n");
    bool all_passed = true;
    int passed = 0;
    float max = 2.0f;
    float min = -2.0f;

    srand(time(NULL));
    for (int iterations = 0; iterations < trials; iterations++) {
        if (debug && iterations % 100 == 0) {
            printf("Randomisiert: %d/%d...\n", iterations, trials);
        }

        bool color = rand() % 2 == 0;
        size_t width = 1 + rand() % 2000;
        size_t height = 1 + rand() % 2000;
        ssize_t height_signed = (rand() & 1) ? -height : height;
        unsigned n = 1 + rand() % 512;
        float start_real = min + (max - min) * ((float) rand()/ (float) RAND_MAX);
        float start_imag = min + (max - min) * ((float) rand()/ (float) RAND_MAX);
        float complex start = start_real + start_imag * I;
        float res = 0.0001f + 100.0f * ((float) rand()/ (float) RAND_MAX);

        TestCase tc = {start, width, height_signed, res, n, color, "bruteforce"};
        if (correctnessTest(tc, false)) {
            passed++;
        } else {
            all_passed = false;
        }
    }
    printf("Randomisierte Tests sind fertig. Erfolg bei %d/%d\n\n", passed, trials);
    return all_passed;
}

bool symmetryTest(size_t width, ssize_t height_signed, int trials, bool debug) {
    /* Führt viele randomisierte Tests aus, die gezielt das center auf einem geraden y setzen, da dann symmetrie
     * funktionieren muss. Dann wählen wir zufällig sehr kleine resolutions, da dabei unter Symmetrie immer
     * Probleme aufgetreten sind*/
    printf("Starte Symmetrie Tests\n");
    bool all_passed = true;
    int passed = 0;

    // Betrag der Bildhöhe bestimmen und Vorzeichen für die Bildausrichtung in 'factor' speichern
    size_t height = abs_size(height_signed);
    float factor = height_signed < 0 ? -1.0f : 1.0f;

    unsigned n = 100;
    bool color = false;
    srand(time(NULL));
    for (int i = 0; i < trials;  i++){
        if (debug && i % 100 == 0) {
            printf("Symmetrie: %d/%d...\n", i, trials);
        }

        int center = height / 2 + rand() % 10;

        // Kleine zufällige Auflösung wählen. Somit entstehen Rundungsfehler,
        // sodass gespiegelte Koordinaten nicht den exakt gleichen Imaginärteil haben
        float res = 0.002f + 0.011f * ((float) rand()/(float) RAND_MAX);

        // Zufälligen Startwert für den Realteil wählen.
        float start_real = -2.0f + 4.0f * ((float) rand()/ (float) RAND_MAX);

        // Imaginärteil so berechnen, dass der durch 'center' gewählte
        // Mittelpunkt auf der Symmetrieachse liegt
        float start_imag = -(center * factor * res);

        float complex start = start_real + start_imag * I;

        TestCase tc = {start, width, height_signed, res, n, color, "symmetrie"};
        if (correctnessTest(tc, false)) {
            passed++;
        } else {
            all_passed = false;
        }
    }
    printf("Symmetrie Tests sind fertig. Erfolg bei %d/%d\n\n", passed, trials);
    return all_passed;
}

bool testSuite() {
    // Prüft zuerst die Korrektheit und benchmarked dann die Versionen
    printf("Starte Korrektheits Tests:\n\n");
    bool print_debug = true;
    bool print_time_debug = true;
    bool result = individualTests(print_debug) ? true : false;
    result = randomizedTest(200, print_time_debug) ? result : false;
    result = symmetryTest(200, 200, 250, print_time_debug) ? result : false;
    printf("Korrektheits Tests beendet\n");

    printf("\n\n---------------------------------------\n\n");

    printf("Starte Benchmarking (mit steigender Bildgröße):\n\n");
    /* Wir starten bei dem TestCase tc und inkrementieren dann Stück für Stück die Breite/Höhe, sodas wir die
     * Performance der Versionen bei größeren Rechenaufwand beobachten können.
     */
    float complex start = -2.0f - 2.0f * I;
    size_t width = 100;
    unsigned n = 256;
    bool color = false;

    printf("Der Benchmark misst die Laufzeit aller Versionen bei steigender (quadratischer) Bildgröße und "
           "angepasster Auflösung.\n");
    printf("Startkoordinate: %f %fI\n", crealf(start), cimag(start));
    printf("Startlänge: %zu (wird erhöht)\n", width);
    printf("Iterationen: %d\n", n);
    printf("Bild ist grau\n\n");

    size_t steps = 5;
    TestCase startTestCase = {start, width, width, 0.0f, n, color, "plot"};
    size_t step_length = 200;

    generateCustomPlottableTable(steps, startTestCase, step_length);
    printf("Benchmarking beendet:\n\n");

    return result;
}
