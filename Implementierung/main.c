#include <stdio.h>
#include <complex.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "NaiveImplementation/tricorn_V1.h"
#include "Simd/tricorn_V2.h"
#include "Sym/tricorn_V3.h"
#include "Circle/tricorn_V4.h"
#include "LatencyThroughput/tricorn_V5.h"
#include "bitmap.h"

#include "MultiThreading/tricorn.h"
#include "testSuite.h"
#include "helpMessage.h"
#include "correctnesTests.h"
#include <errno.h>
#include <stdbool.h>


int main(int argc, char *argv[]) {
    bool isBenchmark = false;
    ssize_t benchmark_iterations = 1;
    unsigned v = 0;
    float complex start = -2 + 2*I; //start parameter
    float real;
    float imag;
    size_t width = 4000;
    ssize_t width_signed = 4000;
    ssize_t height_signed = -4000;
    ssize_t n1 = 256;
    unsigned n = 256;
    bool color = false;
    char *filename;
    float res = 4.0f/(float)width;
    bool resWasSet = false;
    char new_filename[256];
    snprintf(new_filename, 256, "%s.bmp", "test");

    char *end;
    char *end2;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-correct") == 0) {
            if (BitmapVergleichTests()) {
                printf("Alle Bitmaps gleich\n");
            }
            else {
                printf("Fehler Im Code\n");
            }
            return 0;
        }
        // Graph erstellen
        else if (strcmp(argv[i], "-graph") == 0) {
            if (generatePlottableTable() == 1) {
                fprintf(stderr, "Fehler beim Erstellen des Graphen\n");
                return 1;
            }
            return 0;
        }
        // Zeit messen
        else if (strcmp(argv[i], "--test") == 0 || strcmp(argv[i], "-t") == 0) {
            if (testSuite() == 1) {
                fprintf(stderr, "Fehler beim Ausführen der Testsuite\n");
                return 1;
            }
            return 0;
        }
        // Help Message
        else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            helpMessage();
            return 0;
        }
        // B
        else if (strcmp(argv[i], "-B") == 0) {
            if (i+1>=argc) {
                fprintf(stderr, "Error: Fehlendes Argument\n");
                return 1;
            }
            errno = 0;
            ssize_t tempIteration = benchmark_iterations = strtol(argv[i+1], &end, 10);
            isBenchmark = true;
            if (*end != '\0' || tempIteration <= 0||errno != 0) {
                fprintf(stderr, "Error: nach -B ist kein gültiger Int für die Anzahl an Wiederholungen\n");
                return 1;
            }
            i++;
        }
        // v
        else if (strcmp(argv[i], "-V") == 0) {
            long tempV;
            if (i+1>=argc) {
                fprintf(stderr, "Error: Fehlendes Argument\n");
                return 1;
            }
            errno = 0;
            tempV = strtol(argv[i+1], &end, 10);
            if (*end != '\0'||argv[i+1]==end||errno!=0||tempV<0||tempV>5) {
                fprintf(stderr, "Error: nach -V muss eine Zahl zwischen 0 und 5 sein\n");
                return 1;
            }
            v = (unsigned)tempV;
            i++;
        }
        //start parameter; bestimmen durch -s <REALTEIL> <IMAGINAERTEIL>; beide müssen hierbei gültige floats sein
        else if (strcmp(argv[i], "-s") == 0) {
            if (i+1>=argc) {
                fprintf(stderr, "Error: Fehlendes Argument\n");
                return 1;
            }
            errno = 0;

            real = strtof(argv[i+1], &end);
            if (*end != ','||errno != 0||argv[i+1]==end) {
                fprintf(stderr, "Error: nach -s ist kein gültiger Float für den realteil\n");
                return 1;
            }
            errno = 0;
            imag = strtof(end+1, &end2);
            if (*end2 != '\0'||errno != 0||end+1==end2) {
                fprintf(stderr, "Error: nach -s ist kein gültiger Float für den imaginärteil\n");
                return 1;
            }
            start = real + imag * I;
            i++;
        }
        //breite+höhe mit -d <BREITE> <HÖHE>, breite muss hierbei ssize_t folgen und höhe ssize_t
        else if (strcmp(argv[i], "-d") == 0) {
            if (i+1>=argc) {
                fprintf(stderr,"Error: Fehlendes Argument\n");
                return 1;
            }
            errno = 0;
            width_signed = strtol(argv[i+1], &end, 10);
            if (errno != 0 || *end != ','||argv[i+1]==end) {
                fprintf(stderr,"ungültiges Format\n");
                return 1;
            }
            errno = 0;
            height_signed = strtol(end+1, &end2, 10);
            if (errno != 0 || *end2 != '\0'||end+1==end2) {
                fprintf(stderr,"Ungültiges Format\n");
                return 1;
            }
            if (width_signed <= 0) {
                fprintf(stderr, "Error: width nicht positiv\n");
                return 1;
            }
            if (height_signed == 0) {
                fprintf(stderr, "height = 0\n");
                return 1;
            }
            width = width_signed ;
            i++;
        }
        //resolution (auch frei wählbar) mit -r <FLOAT>; <FLOAT> muss hierbei ein Float sein
        else if (strcmp(argv[i], "-r") == 0) {
            if (i+1>=argc) {
                fprintf(stderr, "Error: Fehlendes Argument\n");
                return 1;
            }
            errno = 0;
            float tempRes = strtof(argv[i+1], &end);

            if (*end != '\0'||errno != 0||argv[i+1]==end||tempRes<=0.0f) {
                fprintf(stderr, "Error: nach -r ist keine gültige Zahl für <RESOLUTION>\n");
                return 1;
            }
            res = tempRes;
            resWasSet = true;
            i++;
        }
        //n bestimmen durch -n <ZAHL> Flag; <ZAHL> muss hierbei gültige, nicht negative Zahl sein
        else if (strcmp(argv[i], "-n") == 0) {
            if (i+1>=argc) {
                fprintf(stderr, "Error: Fehlendes Argument\n");
                return 1;
            }
            errno = 0;

            n1 = strtol(argv[i+1], &end, 10);
            if (*end != '\0'||errno != 0||argv[i+1]==end) {
                fprintf(stderr, "Error: nach -n ist keine gültige Zahl\n");
                return 1;
            }
            if (n1<0) {
                fprintf(stderr, "Error: n negativ\n");
                return 1;
            }
            n = (unsigned)n1;
            i++;
        }
        //color setzen per -C oder --color flag
        //false = Grauton berechnen durch Formel / true = Farbpalette
        else if (strcmp(argv[i], "--color") == 0 || strcmp(argv[i], "-C") == 0) {
            color = true;
        }
        //Dateiname angeben, falls gegeben
        else if (strcmp(argv[i], "-o") == 0) {
            if (i+1>=argc) {
                fprintf(stderr, "Error: Fehlendes Argument\n");
                return 1;
            }
            filename = argv[i+1];
            i++;
            size_t nameLength = strlen(filename);

            if (nameLength < 4 || strcmp(filename+nameLength-4, ".bmp") != 0) {

                snprintf(new_filename, 256, "%s.bmp", filename);
            }
            else {
                snprintf(new_filename, 256, "%s", filename);
            }
        }
        else {
            fprintf(stderr, "Error: Unbekannte Flags %s\n", argv[i]);
            return 1;
        }
    }
    //Nun sind alle Parameter aus den Flags gesetzt
    if (!resWasSet) {
        res = 4.0f / (float)width;
    }


    size_t height = height_signed < 0 ? -(size_t) height_signed : (size_t) height_signed;;

    const size_t bytesPerPixel = color ? 3 : 1;
    size_t row_size = ((width * bytesPerPixel+3)/4)*4;
    size_t pixelArraySize = row_size * height;

    unsigned char *img = malloc(pixelArraySize);
    if (img == NULL) {
        fprintf(stderr, "Error allocating memory\n");
        return 1;
    }

    struct timespec time_start; // Zeitmessung (siehe Folien Benchmarking)
    if (isBenchmark) {
        clock_gettime(CLOCK_MONOTONIC, &time_start);
    }

    if (v == 1) {
        for (ssize_t i=0; i < benchmark_iterations; i++) { // Ohne Benchmarking nur eine Iteration
            tricorn_V1(start, width, height_signed, res, n, color, img);
        }
    } else if (v == 2) {
        for (ssize_t i=0; i < benchmark_iterations; i++) {
            tricorn_V2(start, width, height_signed, res, n, color, img);
        }
    } else if (v == 3) {
        for (ssize_t i=0; i < benchmark_iterations; i++) {
            tricorn_V3(start, width, height_signed, res, n, color, img);
        }
    } else if (v == 4) {
        for (ssize_t i=0; i < benchmark_iterations; i++) {
            tricorn_V4(start, width, height_signed, res, n, color, img);
        }
    } else if (v == 5) {
        for (ssize_t i=0; i < benchmark_iterations; i++) {
            tricorn_V5(start, width, height_signed, res, n, color, img);
        }
    } else if (v == 0) {
            for (ssize_t i=0; i < benchmark_iterations; i++) {
                tricorn(start, width, height_signed, res, n, color, img);
            }
    } else {
        fprintf(stderr, "Error: V muss in [0;5] sein\n");
        free(img);
        return 1;
    }


    if (isBenchmark) {  // Zeitmessung (siehe Folien Benchmarking)
        struct timespec time_end;
        clock_gettime(CLOCK_MONOTONIC, &time_end);
        const double time = (double)(time_end.tv_sec - time_start.tv_sec) + 1e-9 * (time_end.tv_nsec - time_start.tv_nsec);
        if (time < 1) { // unter 1 Sekunde getestet (ungültig)
            fprintf(stderr, "Warnung: Gesamtlaufzeit liegt unter 1sek. Erhöhe die Iterationen! (%fs < 1s)\n", time);
        }
        double avg_time = time / (double)benchmark_iterations;
        printf("Gesamtlaufzeit: %fs bei %zu Wiederholungen\n", time, benchmark_iterations);
        printf("Dauer pro Iteration: %fs\n", avg_time); //%g für ganz kleine Zahlen?
    }

    int ret = writeBmp(new_filename, width, height_signed, img, color);

    free(img);
    return ret;
}