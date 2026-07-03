#include <stdio.h>
#include <complex.h>
#include "NaiveImplementation/bitmap.h"
#include "NaiveImplementation/escapetime.h"
#include "Simd/bitmap.h"
#include "Simd/escapetime.h"
#include <complex.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "Sym/bitmap.h"
#include "Circle/bitmap.h"

/*
int main() {
    const unsigned limit = 10;
    const float complex exampleStart = 0;
    const float complex exampleC = 0.0 + 0.0 * I;


    const unsigned result = escapeTime(limit, exampleStart, exampleC);
    if (limit == result) {
        // besser:? "Did not escape in %u iterations\n", limit
        printf("Did not converge :(");
    } else {
        //warum kein %u?
        printf("Converged after %d (Limit: %d)", result, limit);

    }
    return 0;
}
*/

int main(int argc, char *argv[]) {
    char *end;
    char *end2;

    /* -s <REALTEIL> <IMAGINÄRTEIL> ; beide in float
     * -d <WIDTH> <HEIGHT>; width positiver Integer; height auch negativ
     * -n <MAXITERATIONS> muss Integer sein und positiv
     * -r <SCHRITTWEITE PRO PIXEL> muss positive float sein
     * -o <NAME> name ohne .bmp angeben
     * -C || --color einfach setzen, dann wird bild mit Eintracht Braunschweig Farben ausgegeben
     *
     * -time ; Printed Durchschnittslaufzeit der Berechnung auf der Console. Kann mit anderen Parametern übergeben werden
     */


    //time parameter
    bool time_parameter = false;
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-time") == 0) {
            time_parameter = true;
        }
    }


    //v
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

    //start parameter; bestimmen dur -s <REALTEIL> <IMAGINAERTEIL>; beide müssen hierbei gültige floats sein
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
            if (i+2>=argc) {
                printf("Error: Fehlendes Argument\n");
                return 1;
            }
            widthsigned = strtol(argv[i+1], &end, 10);
            if (*end != '\0') {
                printf("Error: nach -d ist keine gültige Zahl f+r <WIDTH>\n");
                return 1;
            }
            heightsigned = strtol(argv[i+2], &end, 10);
            if (*end != '\0') {
                printf("Error: nach -d ist keine gültige Zahl für <HEIGHT>\n");
                return 1;
            }
        }
    }
    if (widthsigned < 0) {
        printf("Error: width negativ\n");
        return 1;
    }
    width = widthsigned ;


    //resolution (auch frei wählbar) mit -r <FLOAT>; <FLOAT> muss hierbei ein FLoat sein
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
            res = res;
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
        height = heightsigned * (-1);
    }
    else {
        height = heightsigned ;
    }

    size_t row_size = ((width * 3+3)/4)*4;
    size_t pixelArraySize = row_size * height;

    unsigned char *img = malloc(pixelArraySize);
    if (img == NULL) { //Fehler
        printf("Error allocating memory\n");
        return 1;
    }

    if (time_parameter) {

        //Naive Implementierung
        const unsigned iterations_naive = 25;

        struct timespec time_start_naive; // Zeitmessung start (siehe Folien Benchmarking)
        clock_gettime(CLOCK_MONOTONIC, &time_start_naive);

        for (unsigned i=0; i < iterations_naive; i++) {
            genImage(start, width, heightsigned, res, n, color, img);
        }

        struct timespec time_end_naive; // Zeitmessung ende (siehe Folien Benchmarking)
        clock_gettime(CLOCK_MONOTONIC, &time_end_naive);
        double time_naive = time_end_naive.tv_sec - time_start_naive.tv_sec + 1e-9 * (time_end_naive.tv_nsec - time_start_naive.tv_nsec);
        if (time_naive < 1) { //unter 1 Sekunde getestet
            printf("Error: SIMD Implementierung hat zu wenig Iterationen (Gesamtlaufzeit war unter 1 Sekunde)\n");
            return 1;
        }
        double avg_time_naive = time_naive / iterations_naive;
        printf("Naive Implementierung: %f sec pro Iteration (%u Iterationen)\n", avg_time_naive, iterations_naive);

        char Dateiname_naive[256] = "Naive.bmp";
        writeBmp(Dateiname_naive, width, heightsigned, img);



        //SIMD Implementierung
        const unsigned iterations_simd = 75;

        struct timespec time_start_simd; // Zeitmessung start (siehe Folien Benchmarking)
        clock_gettime(CLOCK_MONOTONIC, &time_start_simd);

        for (unsigned i=0; i < iterations_simd; i++) {
            genImageSIMD(start, width, heightsigned, res, n, color, img);
        }

        struct timespec time_end_simd; // Zeitmessung ende (siehe Folien Benchmarking)
        clock_gettime(CLOCK_MONOTONIC, &time_end_simd);
        double time_simd = time_end_simd.tv_sec - time_start_simd.tv_sec + 1e-9 * (time_end_simd.tv_nsec - time_start_simd.tv_nsec);
        if (time_simd < 1) { //unter 1 Sekunde getestet
            printf("Error: SIMD Implementierung hat zu wenig Iterationen (Gesamtlaufzeit war unter 1 Sekunde)\n");
            return 1;
        }
        double avg_time_simd = time_simd / iterations_simd;
        printf("SIMD Implementierung: %f sec pro Iteration (%u Iterationen)\n", avg_time_simd, iterations_simd);

        char Dateiname_simd[256] = "SIMD.bmp";
        writeBmpSIMD(Dateiname_simd, width, heightsigned, img);

        // Sym Implementierung
        const unsigned iterations_sym = 75;

        struct timespec time_start_sym; // Zeitmessung start (siehe Folien Benchmarking)
        clock_gettime(CLOCK_MONOTONIC, &time_start_sym);

        for (unsigned i = 0; i < iterations_sym; i++) {
            symGenImage(start, width, heightsigned, res, n, color, img);
        }

        struct timespec time_end_sym; // Zeitmessung ende (siehe Folien Benchmarking)
        clock_gettime(CLOCK_MONOTONIC, &time_end_sym);
        double time_sym = time_end_sym.tv_sec - time_start_sym.tv_sec + 1e-9 * (time_end_sym.tv_nsec - time_start_sym.tv_nsec);
        if (time_sym < 1) { // unter 1 Sekunde getestet
            printf("Error: Sym Implementierung hat zu wenig Iterationen (Gesamtlaufzeit war unter 1 Sekunde)\n");
            return 1;
        }
        double avg_time_sym = time_sym / iterations_sym;
        printf("Sym Implementierung: %f sec pro Iteration (%u Iterationen)\n", avg_time_sym, iterations_sym);

        char Dateiname_sym[256] = "Sym.bmp";
        symWriteBmp(Dateiname_sym, width, heightsigned, img);

        // Circle Implementierung
        const unsigned iterations_circle = 75;

        struct timespec time_start_circle; // Zeitmessung start (siehe Folien Benchmarking)
        clock_gettime(CLOCK_MONOTONIC, &time_start_circle);

        for (unsigned i = 0; i < iterations_circle; i++) {
            genImageCircle(start, width, heightsigned, res, n, color, img);
        }

        struct timespec time_end_circle; // Zeitmessung ende (siehe Folien Benchmarking)
        clock_gettime(CLOCK_MONOTONIC, &time_end_circle);
        double time_circle = time_end_circle.tv_sec - time_start_circle.tv_sec + 1e-9 * (time_end_circle.tv_nsec - time_start_circle.tv_nsec);
        if (time_circle < 1) { // unter 1 Sekunde getestet
            printf("Error: Circle Implementierung hat zu wenig Iterationen (Gesamtlaufzeit war unter 1 Sekunde)\n");
            return 1;
        }
        double avg_time_circle = time_circle / iterations_circle;
        printf("Circle Implementierung: %f sec pro Iteration (%u Iterationen)\n", avg_time_circle, iterations_circle);

        char Dateiname_circle[256] = "Circle.bmp";
        writeBmpCircle(Dateiname_circle, width, heightsigned, img);

    } else {
        if (v == 0) {
            genImage(start, width, heightsigned, res, n, color, img);
            writeBmp(neuerDateiname, width, heightsigned, img);
        } else if (v == 1) {
            genImageSIMD(start, width, heightsigned, res, n, color, img);
            writeBmpSIMD(neuerDateiname, width, heightsigned, img);
        } else if (v == 2) {
            symGenImage(start, width, heightsigned, res, n, color, img);
            symWriteBmp(neuerDateiname, width, heightsigned, img);
        }  else if (v == 3) {
            genImageCircle(start, width, heightsigned, res, n, color, img);
            writeBmpCircle(neuerDateiname, width, heightsigned, img);
        } else {
            printf("Error: v muss in [0;3] sein\n");
            free(img);
            return 1;
        }
    }

    free(img);
    return 0;
}

/* Betriebsanleitung:
 * gcc Implementierung/NaiveImplementation/main.c Implementierung/NaiveImplementation/bitmap.c Implementierung/NaiveImplementation/escapetime.c -o test
 * ./test
 * explorer.exe test.bmp
 */