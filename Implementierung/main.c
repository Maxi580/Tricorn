#include <stdio.h>
#include <complex.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <errno.h>
#include <limits.h>

#include "NaiveImplementation/tricorn_V1.h"
#include "Simd/tricorn_V2.h"
#include "Circle/tricorn_V3.h"
#include "LatencyThroughput/tricorn_V4.h"
#include "bitmap.h"

#include "MultiThreading/tricorn.h"
#include "testSuite.h"
#include "helpMessage.h"
#include "utils.h"
#include "graph.h"


int main(int argc, char *argv[]) {

    // Aktiviert oder deaktiviert den Benchmark-Modus (standardmäßig auf false).
    bool isBenchmark = false;

    // Anzahl der Durchläufe im Benchmark-Modus.
    ssize_t benchmark_iterations = 1;

    // Ausgewählte Implementierungsvariante.
    // Standardmäßig wird die Multithreading-Implementierung (V0) verwendet.
    unsigned v = 0;

    // Standardmäßiger Startpunkt des dargestellten Bereiches
    float complex start = -2 + 2 * I;

    float real;
    float imag;

    // Standardmäßige Bildbreite.
    size_t width = 4000;
    ssize_t width_signed = 4000;

    // Standardmäßige Höhe.
    ssize_t height_signed = -4000;


    //Standardmäßige maximale Iterationszahl.
    unsigned n = 256;
    ssize_t n1;

    // Standardmäßig wird ein Graustufenbild.
    bool color = false;

    char *filename;

    // Standardmäßige Auflösung des dargestellten Bereichs.
    float res = 4.0f/(float)width;

    // Gibt an, ob die Auflösung explizit mit der -r Flag gesetzt wurde.
    bool resWasSet = false;

    // Standardmäßiger Dateiname.
    char new_filename[256];
    snprintf(new_filename, 256, "%s.bmp", "test");

    char *end;
    char *end2;

    // Verarbeitet alle übergebenen Kommandozeilenargumente.
    for (int i = 1; i < argc; i++) {

        // Führt bei gesetzter Flag die vollständige TestSuite aus.
        if (strcmp(argv[i], "--test") == 0 || strcmp(argv[i], "-t") == 0) {
            if (testSuite()) {
                return EXIT_SUCCESS;
            }
            return EXIT_FAILURE;
        }

        // Erzeugt Messdaten für einen Performance-Graphen bei gesetzter Flag.
        else if (strcmp(argv[i], "-graph") == 0) {
            if (generatePlottableTable() == 1) {
                fprintf(stderr, "Fehler beim Erstellen des Graphen\n");
                return EXIT_FAILURE;
            }
            return EXIT_SUCCESS;
        }

        // Gibt bei gesetzter Flag die Hilfe zur Verwendung des Programms aus.
        else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            helpMessage();
            return EXIT_SUCCESS;
        }
        // Aktiviert den Benchmark-Modus und legt Anzahl der Wiederholungen fest.
        else if (strcmp(argv[i], "-B") == 0) {

            // Prüft, ob hinter -B ein weiteres Argument vorhanden ist.
            if (i+1>=argc) {
                fprintf(stderr, "Error: Fehlendes Argument hinter -B\n");
                return EXIT_FAILURE;
            }
            errno = 0;

            // Liest die Anzahl an Benchmark-Wiederholungen ein.
            ssize_t tempIteration = benchmark_iterations = strtol(argv[i+1], &end, 10);
            isBenchmark = true;

            // Das Argument muss eine vollständige gültige,
            // positive Ganzzahl sein.
            if (*end != '\0' || tempIteration <= 0||errno != 0) {
                fprintf(stderr, "Error: nach -B ist kein gültiger Int für die Anzahl an Wiederholungen\n");
                return EXIT_FAILURE;
            }

            // Überspringt das bereits eingelesene Argument hinter -B
            i++;
        }

        // Wählt die gewünschte Implementierunsvariante aus
        else if (strcmp(argv[i], "-V") == 0) {
            long tempV;

            // Prüft, ob hinter -V ein weiteres Argument vorhanden ist.
            if (i+1>=argc) {
                fprintf(stderr, "Error: Fehlendes Argument hinter -V\n");
                return EXIT_FAILURE;
            }
            errno = 0;

            // Liest die gewünschte Implementierungsnummer ein.
            tempV = strtol(argv[i+1], &end, 10);

            // Die Eingabe muss eine ganze Zahl zwischen 0 und 4 sein.
            if (*end != '\0'||argv[i+1]==end||errno!=0||tempV<0||tempV>5) {
                fprintf(stderr, "Error: nach -V muss eine Zahl zwischen 0 und 4 sein\n");
                return EXIT_FAILURE;
            }

            // Überspringt das bereits eingelesene Argument hinter -V
            v = (unsigned)tempV;


            i++;
        }

        // Liest den Startpunkt des dargestellten Bereichs ein.
        // Das erwartete Format lautet: -s <REALTEIL>,<IMAGINÄRTEIL>.
        else if (strcmp(argv[i], "-s") == 0) {

            // Prüft, ob hinter -s ein weiteres Argument vorhanden ist.
            if (i+1>=argc) {
                fprintf(stderr, "Error: Fehlendes Argument hinter -s\n");
                return EXIT_FAILURE;
            }

            errno = 0;

            // Liest den Realteil bis zum Komma ein.
            real = strtof(argv[i+1], &end);

            //Der Realteil muss gültig sein und es muss direkt danach ein Komma folgen.
            if (*end != ','|| errno != 0 || argv[i+1] == end) {
                fprintf(stderr, "Error: Nach -s ist kein gültiger Float für den Realteil\n");
                return EXIT_FAILURE;
            }
            errno = 0;

            // Liest den Imaginärteil nach dem Komma ein.
            imag = strtof(end+1, &end2);

            // Der Imaginärteil muss gültig sein.
            if (*end2 != '\0'|| errno != 0 ||end+1==end2) {
                fprintf(stderr, "Error: Nach -s ist kein gültiger Float für den Imaginärteil\n");
                return EXIT_FAILURE;
            }

            // Setzt den komplexen Startpunkt aus Real- und Imaginärteil zusammen.
            start = real + imag * I;

            // Überspringt das bereits ausgelesene Argument nach -s.
            i++;
        }

        // Liest Bildbreite und Bildhöhe ein.
        // Das erwartete Format lautet -d <BREITE>,<HÖHE>.
        else if (strcmp(argv[i], "-d") == 0) {

            // Prüft, ob hinter -d ein weiteres Argument vorhanden ist.
            if (i+1>=argc) {
                fprintf(stderr, "Error: Fehlendes Argument hinter -d\n");
                return EXIT_FAILURE;
            }

            errno = 0;

            // Liest die Bildbreite bis zum Komma ein.
            width_signed = strtol(argv[i+1], &end, 10);

            // Die Breite muss eine gültige Ganzzahl sein und es muss direkt darauf ein Komma folgen.
            if (errno != 0 || *end != ','||argv[i+1]==end) {
                fprintf(stderr, "Error: Ungültiges Format hinter -d\n");
                return EXIT_FAILURE;
            }

            errno = 0;

            // Liest die Bildhöhe nach dem Komma ein.
            height_signed = strtol(end+1, &end2, 10);

            // Die Höhe muss eine gültige Ganzzahl sein.
            if (errno != 0 || *end2 != '\0'||end+1==end2) {
                fprintf(stderr, "Error: Ungültiges Format hinter -d\n");
                return EXIT_FAILURE;
            }

            // Die Bildbreite muss positiv sein und in den Wertebereich des BMP-Formats passen.
            if (width_signed <= 0 || width_signed > BMP_MAX_RANGE) {
                fprintf(stderr, "Error: Width muss positiv sein und in ]0,  2.147.483.647] liegen. \n");
                return EXIT_FAILURE;
            }

            // Die Bildhöhe darf nicht null sein und muss sin den zulässigen Wertebereich des BMP-Formats passen.
            if (height_signed == 0 || height_signed > BMP_MAX_RANGE || height_signed < BMP_MIN_RANGE) {
                fprintf(stderr, "Error: Height darf nicht 0 sein und muss in [-2.147.483.648, 2.147.483.647] "
                                "liegen\n");
                return EXIT_FAILURE;
            }
            // Die vorherige Bereichsprüfung verhindert eine ungültige Konvertierung in size_t.
            width = width_signed;

            // Überspringt das bereits ausgelesene Argument nach -d.
            i++;
        }

        // Legt die Auflösung des dargestellten Bereichs fest.
        // Das erwartete Format lautet: -r <AUFLÖSUNG>
        else if (strcmp(argv[i], "-r") == 0) {

            // Prüft, ob hinter -r ein weiteres Argument vorhanden ist.
            if (i+1>=argc) {
                fprintf(stderr, "Error: Fehlendes Argument hinter -r\n");
                return EXIT_FAILURE;
            }

            errno = 0;

            // Liest die angegebene Auflösung als Gleitkommazahl ein.
            float tempRes = strtof(argv[i+1], &end);

            // Die Auflösung muss gültig sein und größer als null sein.
            if (*end != '\0'||errno != 0||argv[i+1]==end||tempRes<=0.0f) {
                fprintf(stderr, "Error: nach -r ist keine gültige Zahl für <RESOLUTION>\n");
                return EXIT_FAILURE;
            }

            // übernimmt die eingegebene Auflösung
            res = tempRes;

            // Verhindert, dass später der Standardwert berechnet wird.
            resWasSet = true;

            // Überspringt das bereits ausgelesene Argument nach -r.
            i++;
        }

        // Legt die maximale Anzahl der Iterationen fest.
        // Das erwartete Format lautet -n <ANZAHL>
        else if (strcmp(argv[i], "-n") == 0) {

            // Prüft, ob hinter -n ein weiteres Argument vorhanden ist.
            if (i+1>=argc) {
                fprintf(stderr, "Error: Fehlendes Argument hinter -n\n");
                return EXIT_FAILURE;
            }

            errno = 0;

            // Liest die maximale Iterationszahl ein.
            n1 = strtol(argv[i+1], &end, 10);

            // Die Eingabe muss eine vollständige Ganzzahl sein.
            if (*end != '\0'|| errno != 0|| argv[i+1]==end) {
                fprintf(stderr, "Error: nach -n ist keine gültige Zahl\n");
                return EXIT_FAILURE;
            }

            // Negative Iterationszahlen sind nicht zulässig.
            if (n1 < 0) {
                fprintf(stderr, "Error: n muss positiv sein\n");
                return EXIT_FAILURE;
            }

            // Der eingelesene Wert muss in den Wertebereich des Datentyps 'unsigned' passen.
            if (n1 > UINT_MAX) {
                fprintf(stderr, "Error: n zu groß (maximal %u)\n", UINT_MAX);
                return EXIT_FAILURE;
            }

            // übernimmt die neue Iterationszahl.
            n = (unsigned) n1;

            // Überspringt das bereits ausgelesene Argument nach -n.
            i++;
        }

        // Aktiviert die farbige Darstellung.
        // Ohne diese Flag wird ein Graustufenbild erzeugt.
        else if (strcmp(argv[i], "--color") == 0 || strcmp(argv[i], "-C") == 0) {
            color = true;
        }

        // Legt den Namen der Ausgabedatei fest.
        // Das erwartete Format lautet: -o <DATEINAME>
        else if (strcmp(argv[i], "-o") == 0) {

            // Prüft, ob hinter -o ein weiteres Argument vorhanden ist.
            if (i+1>=argc) {
                fprintf(stderr, "Error: Fehlendes Argument hinter -o\n");
                return EXIT_FAILURE;
            }

            // übernimmt den angegebenen Dateinamen.
            filename = argv[i+1];

            // Überspringt das bereits ausgelesene Argument nach -o.
            i++;

            size_t nameLength = strlen(filename);

            // Fügt die Endung '.bmp' hinzu, falls sie noch nicht vorhanden ist.
            if (nameLength < 4 || strcmp(filename+nameLength-4, ".bmp") != 0) {
                snprintf(new_filename, 256, "%s.bmp", filename);
            }
            else {
                // Übernimmt andernfalls den Dateinamen unverändert, da der Name schon auf '.bmp' endet.
                snprintf(new_filename, 256, "%s", filename);
            }
        }

        // Bricht die Verarbeitung bei einem unbekannten Argument ab.
        else {
            fprintf(stderr, "Error: Unbekannte Flags %s\n", argv[i]);
            return EXIT_FAILURE;
        }
    }

    // Falls keine Auflösung über -r angegeben wurde, wird sie aus der Bildbreite berechnet.
    if (!resWasSet) {
        res = 4.0f / (float)width;
    }

    // Bestimmt die absolute Bildhöhe.
    // Das Vorzeichen legt nur die Ausrichtung der Bitmap fest.
    size_t height = abs_size(height_signed);

    // Bestimmt die Anzahl der Bytes pro Pixel.
    // Farbbilder verwenden drei Bytes, Graustufenbilder nur ein Byte.
    const size_t bytesPerPixel = color ? 3 : 1;

    // Jede BMP-Zeile muss auf ein Vielfaches von vier aufgefüllt werden.
    size_t row_size = ((width * bytesPerPixel+3)/4)*4;


    // Berechnet die Größe des vollständigen Pixelarrays, einschließlich des Paddings.
    size_t pixelArraySize = row_size * height;

    // Bestimmt die Größe der Farbpalette.
    // Farbbilder benötigen keine Palette, Graustufenbilder verwenden eine Palette mit 256 Einträgen.
    size_t paletteSize = color ? COLOR_PALETTE_SIZE : GREY_PALETTE_SIZE;

    // Bestimmt die Position des Pixelarrays innerhalb der BMP-Datei.
    size_t pixelOffset = BMP_HEADER_SIZE + paletteSize;

    // Berechnet die vollständige Größe der BMP-Datei.
    size_t fileSize = pixelOffset + pixelArraySize;

    // Prüft, ob die resultierende Dateigröße die maximal unterstütze BMP-Dateigröße überschreitet.
    if (fileSize > BMP_MAX_FILE_SIZE) {
        fprintf(stderr, "Error: Bitmap datei zu groß (maximal 4 GB)\n");
        return EXIT_FAILURE;
    }

    // Reserviert den Speicher für das berechnete Pixelarray.
    unsigned char *img = malloc(pixelArraySize);

    // Bricht das Programm ab, falls die Speicherreservierung fehlschlägt.
    if (img == NULL) {
        fprintf(stderr, "Error allocating memory\n");
        return EXIT_FAILURE;
    }

    // Speichert bei aktiviertem Benchmark-Modus den Startzeitpunkt der Messung (siehe Folien zur Messung)
    struct timespec time_start;
    if (isBenchmark) {
        clock_gettime(CLOCK_MONOTONIC, &time_start);
    }

    // Führt die ausgewählte Implementierung aus.
    // Im Benchmark-Modus wird die Berechnung mehrfach wiederholt.
    if (v == 1) {
        // Naive Implementierung
        for (ssize_t i=0; i < benchmark_iterations; i++) { // Ohne Benchmarking nur eine Iteration.
            tricorn_V1(start, width, height_signed, res, n, color, img);
        }

    } else if (v == 2) {
        // SIMD-optimierte Implementierung.
        for (ssize_t i=0; i < benchmark_iterations; i++) {
            tricorn_V2(start, width, height_signed, res, n, color, img);
        }

    } else if (v == 3) {
        // Implementierung mit zusätzlicher Kreisprüfung.
        for (ssize_t i=0; i < benchmark_iterations; i++) {
            tricorn_V3(start, width, height_signed, res, n, color, img);
        }

    } else if (v == 4) {
        // Implementierung mit LatencyThroughput.
        for (ssize_t i=0; i < benchmark_iterations; i++) {
            tricorn_V4(start, width, height_signed, res, n, color, img);
        }

    } else if (v == 0) {
        // Standardimplementierung.
        // Verwendet abhängig von der Bildgröße automatisch die Multi-Threading-Variante.
        for (ssize_t i=0; i < benchmark_iterations; i++) {
            tricorn(start, width, height_signed, res, n, color, img);
        }

    } else {
        // Ungültige Implementierung.
        fprintf(stderr, "Error: V muss in [0;4] sein\n");
        free(img);
        return EXIT_FAILURE;
    }

    // Gibt nach Abschluss des Benchmarks die gemessene Laufzeit aus.
    if (isBenchmark) {

        // Zeitmessung (siehe Folien Benchmarking)
        struct timespec time_end;
        clock_gettime(CLOCK_MONOTONIC, &time_end);

        // Berechnet die gesamte Laufzeit in Sekunden.
        const double time = (double)(time_end.tv_sec - time_start.tv_sec) + 1e-9 * (time_end.tv_nsec - time_start.tv_nsec);

        // Sehr kurze Messungen liefern häufig ungenaue Ergebnisse.
        if (time < 1) {
            fprintf(stderr, "Warnung: Gesamtlaufzeit liegt unter 1s. Erhöhe die Iterationen! (%fs < 1s)\n", time);
        }

        // Berechnet die durchschnittliche Laufzeit pro Durchlauf.
        double avg_time = time / (double)benchmark_iterations;
        printf("Gesamtlaufzeit: %fs bei %zd Wiederholungen\n", time, benchmark_iterations);
        printf("Dauer pro Iteration: %fs\n", avg_time); // %g für ganz kleine Zahlen?
    }

    // Schreibt das berechnete Bild als BMP-Datei.
    int ret = writeBmp(new_filename, width_signed, height_signed, img, color);

    // Gibt den reservierten Speicher wieder frei.
    free(img);

    // Gibt den entsprechenden Programmstatus zurück.
    if (ret == 0) {
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}