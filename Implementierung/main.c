#include <stdio.h>
#include <complex.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

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

int main(int argc, char *argv[]) {
    char *end;
    char *end2;

    /* -V <Zahl> ; Versionnummer der Implementierung
     * -B <Zahl> ; Laufzeitmessung der angegebenen Implementierung
     * -s <REALTEIL> <IMAGINÄRTEIL> ; beide in float
     * -d <WIDTH> <HEIGHT>; width positiver Integer; height auch negativ
     * -n <MAXITERATIONS> muss Integer sein und positiv
     * -r <SCHRITTWEITE PRO PIXEL> muss positive float sein
     * -o <NAME> name ohne .bmp angeben
     * -h || --help ; Help Message
     * -C || --color einfach setzen, dann wird bild mit Eintracht Braunschweig Farben ausgegeben
     * -t || --test ; run time tests
     * -graph || erstellt eine Tabelle für Graphen
     * -correct Flag zum Testen der Korrektheit aller Implementierungen, gegen die Naive
     */


    /* CMakeLists.txt falls jmd kopieren will
    cmake_minimum_required(VERSION 4.2)
project(gra26capsproject_t105 C)

set(CMAKE_C_STANDARD 11)

add_executable(gra26capsproject_t105
        Implementierung/NaiveImplementation/tricorn_V4.h
        Implementierung/NaiveImplementation/escapetime.c
        Implementierung/NaiveImplementation/tricorn_V4.c
        Implementierung/NaiveImplementation/escapetime.h
        Implementierung/Simd/tricorn_V4.h
        Implementierung/Simd/escapetime.c
        Implementierung/Simd/tricorn_V4.c
        Implementierung/Simd/escapetime.h
        Implementierung/main.c
        Implementierung/correctnesTests.h
        Implementierung/correctnesTests.c
        Implementierung/bitmap.c
        Implementierung/bitmap.h
        Implementierung/Sym/escapetime.c
        Implementierung/Sym/escapetime.h
        Implementierung/Sym/tricorn_V4.h
        Implementierung/Sym/tricorn_V4.c
        Implementierung/MultiThreading/tricorn_V4.h
        Implementierung/MultiThreading/tricorn_V4.c
        Implementierung/LatencyThroughput/escapetime.c
        Implementierung/LatencyThroughput/escapetime.h
        Implementierung/LatencyThroughput/tricorn_V4.h
        Implementierung/LatencyThroughput/tricorn_V4.c
        Implementierung/Circle/tricorn_V4.h
        Implementierung/Circle/tricorn_V4.c
        Implementierung/Circle/findCircleRad.c
        Implementierung/Circle/tricorn_V4.h
        Implementierung/Circle/tricorn_V4.c
        Implementierung/testSuite.c
        Implementierung/testSuite.h
        Implementierung/Circle/escapetime.c
        Implementierung/Circle/escapetime.h
)


     */

    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-correct") == 0) {
            if (BitmapVergleichTests()) {
                printf("Alle Bitmaps gleich\n");

            }
            else {
                printf("Fehler Im Code\n");
            }
            return 0;
        }

    }

    // Graph Tabelle erstellen
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-graph") == 0) {
            if (genGraphTable() == 1) {
                fprintf(stderr, "Fehler beim Erstellen des Graphen\n");
                return 1;
            }
            return 0;
        }
    }

    // zeit messen
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--test") == 0 || strcmp(argv[i], "-t") == 0) {
            if (testSuite() == 1) {
                fprintf(stderr, "Fehler beim Ausführen der Testsuite\n");
                return 1;
            }
            return 0;
        }
    }

    // Help Message
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            helpMessage();
            return 0;
        }
    }

    // B
    bool isBenchmark = false;
    ssize_t benchmarkIterations = 1;
    for (int i=0; i<argc; i++) {
        if (strcmp(argv[i], "-B") == 0) {
            if (i+1>=argc) {
                fprintf(stderr, "Error: Fehlendes Argument\n");
                return 1;
            }
            ssize_t tempIteration = benchmarkIterations = strtol(argv[i+1], &end, 10);
            isBenchmark = true;
            if (*end != '\0' || tempIteration <= 0) {
                fprintf(stderr, "Error: nach -B ist kein gültiger Int für die Anzahl an Wiederholungen\n");
                return 1;
            }
        }
    }

    // v
    unsigned v = 0;
    for (int i=0; i<argc; i++) {
        if (strcmp(argv[i], "-V") == 0) {
            if (i+1>=argc) {
                fprintf(stderr, "Error: Fehlendes Argument\n");
                return 1;
            }
            v = strtol(argv[i+1], &end, 10);
            if (*end != '\0') {
                fprintf(stderr, "Error: nach -V ist kein gültiger Int für den realteil\n");
                return 1;
            }
        }
    }

    //start parameter; bestimmen durch -s <REALTEIL> <IMAGINAERTEIL>; beide müssen hierbei gültige floats sein
    float complex start = -0 + 0*I; //start parameter
    float realteil;
    float imaginaerteil;
    for (int i=0; i<argc; i++) {
        if (strcmp(argv[i], "-s") == 0) {
            if (i+2>=argc) {
                fprintf(stderr, "Error: Fehlendes Argument\n");
                return 1;
            }
            realteil = strtof(argv[i+1], &end);
            if (*end != '\0') {
                fprintf(stderr, "Error: nach -s ist kein gültiger Float für den realteil\n");
                return 1;
            }
            imaginaerteil = strtof(argv[i+2], &end2);
            if (*end2 != '\0') {
                fprintf(stderr, "Error: nach -s ist kein gültiger Float für den imaginärteil\n");
                return 1;
            }
            start = realteil + imaginaerteil* I;
        }
    }



    //breite+höhe mit -d <BREITE> <HÖHE>, breite muss hierbei ssize_t folgen und höhe ssize_t
    size_t width;
    ssize_t widthsigned = 4000;
    ssize_t heightsigned = -4000;
    for (int i=0; i<argc; i++) {
        if (strcmp(argv[i], "-d") == 0) {
            if (i+1>=argc) {
                printf("Error: Fehlendes Argument\n");
                return 1;
            }
            char *end;
            errno = 0;
            widthsigned = strtol(argv[i+1], &end, 10);
            if (errno != 0 || *end != ',') {
                printf("ungültiges Format\n");
                return 1;
            }
            errno = 0;
            heightsigned = strtol(end+1, &end, 10);
            if (errno != 0 || *end != '\0') {
                printf("Ungültiges Format\n");
                return 1;
            }
        }
    }
    if (widthsigned < 0) {
        fprintf(stderr, "Error: width negativ\n");
        return 1;
    }
    width = widthsigned ;


    //resolution (auch frei wählbar) mit -r <FLOAT>; <FLOAT> muss hierbei ein Float sein
    float res = 4.0f/(float)width;
    for (int i=0; i<argc; i++) {
        if (strcmp(argv[i], "-r") == 0) {
            if (i+1>=argc) {
                fprintf(stderr, "Error: Fehlendes Argument\n");
                return 1;
            }
            res = strtof(argv[i+1], &end);
            if (*end != '\0') {
                fprintf(stderr, "Error: nach -r ist keine gültige Zahl für <RESOLUTION>\n");
                return 1;
            }
            if (res<0) {
                fprintf(stderr, "Error: res negativ\n");
                return 1;
            }
            if (res == 0) {
                fprintf(stderr, "Error: res zero\n");
                return 1;

            }
        }
    }

    //n bestimmen durch -n <ZAHL> Flag; <ZAHL> muss hierbei gültige, nicht negative Zahl sein
    ssize_t n1 = 256;

    for (int i=0; i<argc; i++) {
        if (strcmp(argv[i], "-n") == 0) {
            if (i+1>=argc) {
                fprintf(stderr, "Error: Fehlendes Argument\n");
                return 1;
            }
            n1 = strtol(argv[i+1], &end, 10);
            if (*end != '\0') {
                fprintf(stderr, "Error: nach -n ist keine gültige Zahl\n");
                return 1;
            }
        }
    }
    if (n1<0) {
        fprintf(stderr, "Error: n negativ\n");
        return 1;
    }
    unsigned n = n1;



    //color setzen per -C oder --color flag
    //false = Grauton berechnen durch Formel / true = Farbpalette
    bool color = false;
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--color") == 0 || strcmp(argv[i], "-C") == 0) {
            color = true;
        }
    }

    //Dateiname angeben, falls gegeben
    char *filename = "test";
    for (int i=0; i<argc; i++) {
        if (strcmp(argv[i], "-o") == 0) {
            if (i+1>=argc) {
                fprintf(stderr, "Error: Fehlendes Argument\n");
                return 1;
            }
            filename = argv[i+1];
        }
    }
    char new_filename[256];
    snprintf(new_filename, 256, "%s.bmp", filename);

    size_t height;
    if (heightsigned < 0) {
        height = (size_t)(-heightsigned);
    }
    else {
        height = (size_t)heightsigned ;
    }

    size_t row_size = ((width * 3+3)/4)*4;
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

    if (v == 0) {
        for (ssize_t i=0; i < benchmarkIterations; i++) { // Ohne Benchmarking nur eine Iteration
            tricorn_V1(start, width, heightsigned, res, n, color, img);
        }
    } else if (v == 1) {
        for (ssize_t i=0; i < benchmarkIterations; i++) {
            tricorn_V2(start, width, heightsigned, res, n, color, img);
        }
    } else if (v == 2) {
        for (ssize_t i=0; i < benchmarkIterations; i++) {
            tricorn_V3(start, width, heightsigned, res, n, color, img);
        }
    } else if (v == 3) {
        for (ssize_t i=0; i < benchmarkIterations; i++) {
            tricorn_V4(start, width, heightsigned, res, n, color, img);
        }
    } else if (v == 4) {
        for (ssize_t i=0; i < benchmarkIterations; i++) {
            tricorn_V5(start, width, heightsigned, res, n, color, img);
        }
    } else if (v == 5) {
            for (ssize_t i=0; i < benchmarkIterations; i++) {
                tricorn(start, width, heightsigned, res, n, color, img);
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
        double avg_time = time / (double)benchmarkIterations;
        printf("Gesamtlaufzeit: %fs bei %zu Wiederholungen\n", time, benchmarkIterations);
        printf("Dauer pro Iteration: %fs\n", avg_time); //%g für ganz kleine Zahlen?
    }

    writeBmp(new_filename, width, heightsigned, img);

    free(img);
    return 0;
}

/* Betriebsanleitung:
 * gcc Implementierung/NaiveImplementation/main.c Implementierung/NaiveImplementation/tricorn_V4.c Implementierung/NaiveImplementation/escapetime.c -o test
 * ./test
 * explorer.exe test.bmp
 */