#include <stdio.h>
#include <complex.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bitmap.h"
#include "NaiveImplementation/tricornV0.h"
#include "Simd/tricornV1.h"
#include "Simd/escapetime.h"
#include "Sym/tricornV2.h"
#include "Circle/tricornV3.h"
#include "LatencyThroughput/tricornV4.h"
#include "MultiThreading/tricorn.h"
#include "testSuite.h"
#include "helpMessage.h"
#include "correctnesTests.h"
#include <errno.h>

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
        Implementierung/NaiveImplementation/tricornV3.h
        Implementierung/NaiveImplementation/escapetime.c
        Implementierung/NaiveImplementation/tricornV3.c
        Implementierung/NaiveImplementation/escapetime.h
        Implementierung/Simd/tricornV3.h
        Implementierung/Simd/escapetime.c
        Implementierung/Simd/tricornV3.c
        Implementierung/Simd/escapetime.h
        Implementierung/main.c
        Implementierung/correctnesTests.h
        Implementierung/correctnesTests.c
        Implementierung/bitmap.c
        Implementierung/bitmap.h
        Implementierung/Sym/escapetime.c
        Implementierung/Sym/escapetime.h
        Implementierung/Sym/tricornV3.h
        Implementierung/Sym/tricornV3.c
        Implementierung/MultiThreading/tricornV3.h
        Implementierung/MultiThreading/tricornV3.c
        Implementierung/LatencyThroughput/escapetime.c
        Implementierung/LatencyThroughput/escapetime.h
        Implementierung/LatencyThroughput/tricornV3.h
        Implementierung/LatencyThroughput/tricornV3.c
        Implementierung/Circle/tricornV3.h
        Implementierung/Circle/tricornV3.c
        Implementierung/Circle/findCircleRad.c
        Implementierung/Circle/tricornV3.h
        Implementierung/Circle/tricornV3.c
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
                printf("FehlerImCode\n");
            }
            return 0;
        }

    }

    // Graph Tabelle erstellen
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-graph") == 0) {
            if (genGraphTable() == 1) {
                printf("Fehler beim Erstellen des Graphen\n");
                return 1;
            }
            return 0;
        }
    }

    // zeit messen
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--test") == 0 || strcmp(argv[i], "-t") == 0) {
            if (testSuite() == 1) {
                printf("Fehler beim Ausführen der Testsuite\n");
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
                printf("Error: Fehlendes Argument\n");
                return 1;
            }
            ssize_t tempIteration = benchmarkIterations = strtol(argv[i+1], &end, 10);
            isBenchmark = true;
            if (*end != '\0' || tempIteration <= 0) {
                printf("Error: nach -B ist kein gültiger Int für die Anzahl an Wiederholungen\n");
                return 1;
            }
        }
    }

    // v
    unsigned v = 0;
    for (int i=0; i<argc; i++) {
        if (strcmp(argv[i], "-V") == 0) {
            if (i+1>=argc) {
                printf("Error: Fehlendes Argument\n");
                return 1;
            }
            v = strtol(argv[i+1], &end, 10);
            if (*end != '\0') {
                printf("Error: nach -V ist kein gültiger Int für den realteil\n");
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
                printf("Error: Fehlendes Argument\n");
                return 1;
            }
            realteil = strtof(argv[i+1], &end);
            if (*end != '\0') {
                printf("Error: nach -s ist kein gültiger Float für den realteil\n");
                return 1;
            }
            imaginaerteil = strtof(argv[i+2], &end2);
            if (*end2 != '\0') {
                printf("Error: nach -s ist kein gültiger Float für den imaginärteil\n");
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
            char *erste = strtok(argv[i+1], ",");
            char *zweite = strtok(NULL, ",");

            if (zweite == NULL||erste==NULL) {
                printf("Error: format muss breite, höhe sein");
                return 1;
            }

            errno = 0;
            char *end1;
            widthsigned = (ssize_t) strtol(erste, &end1, 10);
            if (errno != 0||*end1 != '\0') {
                printf("Error: Ungueltige Breite\n");
                return 1;
            }
            errno = 0;
            char *end2;
            heightsigned = (ssize_t) strtol(zweite, &end2, 10);
            if (errno != 0||*end2 != '\0') {
                printf("Error: Ungueltige Höhe\n");
                return 1;
            }



        }
    }
    if (widthsigned < 0) {
        printf("Error: width negativ\n");
        return 1;
    }
    width = widthsigned ;


    //resolution (auch frei wählbar) mit -r <FLOAT>; <FLOAT> muss hierbei ein Float sein
    float res = 4.0f/(float)width;
    for (int i=0; i<argc; i++) {
        if (strcmp(argv[i], "-r") == 0) {
            if (i+1>=argc) {
                printf("Error: Fehlendes Argument\n");
                return 1;
            }
            res = strtof(argv[i+1], &end);
            if (*end != '\0') {
                printf("Error: nach -r ist keine gültige Zahl für <RESOLUTION>\n");
                return 1;
            }
            if (res<0) {
                printf("Error: res negativ\n");
                return 1;
            }
            if (res == 0) {
                printf("Error: res zero\n");
                return 1;

            }
        }
    }

    //n bestimmen durch -n <ZAHL> Flag; <ZAHL> muss hierbei gültige, nicht negative Zahl sein
    ssize_t n1 = 256;

    for (int i=0; i<argc; i++) {
        if (strcmp(argv[i], "-n") == 0) {
            if (i+1>=argc) {
                printf("Error: Fehlendes Argument\n");
                return 1;
            }
            n1 = strtol(argv[i+1], &end, 10);
            if (*end != '\0') {
                printf("Error: nach -n ist keine gültige Zahl\n");
                return 1;
            }
        }
    }
    if (n1<0) {
        printf("Error: n negativ\n");
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
    //TODO .bmp end updaten
    //Dateiname angeben, falls gegeben
    char *dateiname = "test";
    for (int i=0; i<argc; i++) {
        if (strcmp(argv[i], "-o") == 0) {
            if (i+1>=argc) {
                printf("Error: Fehlendes Argument\n");
                return 1;
            }
            dateiname = argv[i+1];
        }
    }
    char neuerDateiname[256];
    snprintf(neuerDateiname, 256, "%s.bmp", dateiname);

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
    if (img == NULL) { //Fehler
        printf("Error allocating memory\n");
        return 1;
    }

    struct timespec time_start; // Zeitmessung (siehe Folien Benchmarking)
    if (isBenchmark) {
        clock_gettime(CLOCK_MONOTONIC, &time_start);
    }

    if (v == 0) {
        for (ssize_t i=0; i < benchmarkIterations; i++) { // Ohne Benchmarking nur eine Iteration
            tricornV0(start, width, heightsigned, res, n, color, img);
        }
    } else if (v == 1) {
        for (ssize_t i=0; i < benchmarkIterations; i++) {
            tricornV1(start, width, heightsigned, res, n, color, img);
        }
    } else if (v == 2) {
        for (ssize_t i=0; i < benchmarkIterations; i++) {
            tricornV2(start, width, heightsigned, res, n, color, img);
        }
    } else if (v == 3) {
        for (ssize_t i=0; i < benchmarkIterations; i++) {
            tricornV3(start, width, heightsigned, res, n, color, img);
        }
    } else if (v == 4) {
        for (ssize_t i=0; i < benchmarkIterations; i++) {
            tricornV4(start, width, heightsigned, res, n, color, img);
        }
    } else if (v == 5) {
            for (ssize_t i=0; i < benchmarkIterations; i++) {
                tricorn(start, width, heightsigned, res, n, color, img);
            }
    } else {
        printf("Error: V muss in [0;5] sein\n");
        free(img);
        return 1;
    }


    if (isBenchmark) {  // Zeitmessung (siehe Folien Benchmarking)
        struct timespec time_end;
        clock_gettime(CLOCK_MONOTONIC, &time_end);
        const double time = (double)(time_end.tv_sec - time_start.tv_sec) + 1e-9 * (time_end.tv_nsec - time_start.tv_nsec);
        if (time < 1) { // unter 1 Sekunde getestet (ungültig)
            printf("Error: Gesamtlaufzeit zu kurz. Erhöhe die Iterationen! (%fs < 1s)\n", time);
            free(img);
            return 1;
        }
        double avg_time = time / (double)benchmarkIterations;
        printf("Dauer pro Iteration: %fs\n", avg_time); //%g für ganz kleine Zahlen?
    }

    writeBmp(neuerDateiname, width, heightsigned, img);

    free(img);
    return 0;
}

/* Betriebsanleitung:
 * gcc Implementierung/NaiveImplementation/main.c Implementierung/NaiveImplementation/tricornV3.c Implementierung/NaiveImplementation/escapetime.c -o test
 * ./test
 * explorer.exe test.bmp
 */