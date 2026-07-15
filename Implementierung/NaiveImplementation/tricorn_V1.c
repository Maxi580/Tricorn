
#include <complex.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <sys/types.h>

#include "escapetime.h"
#include "../colormap.h"

void tricorn_V1(float complex start, size_t width, ssize_t height, float res, unsigned n, bool color, unsigned char* img) {
    // BGR-Bilder brauchen 3 Bytes pro Pixel, Graustufenbilder nur ein Byte.
    int bytesPerPixel = color ? 3 : 1;

    /*
     * Ist 'height' negativ, wird eine Top-Down Bitmap erstellt
     * Die erste gespeicherte Bildzeile ist dabei die oberste Bildzeile.
     */
    if (height < 0) {

        // Für die Schleife wird nur eine positive Höhe benötigt.
        height = height * (-1);

        // Jede BMP Zeile muss eine durch 4 teilbare Anzahl an Bytes besitzen.
        // Fehlende Bytes werden am Ende jeder Zeile durch Padding ergänzt.
        size_t row_size = (((bytesPerPixel*width)+3)/4) * 4;

        //Bildzeilen von oben nach unten durchlaufen.
        for (ssize_t y = 0; y < height; y++) {

            // *row zeigt nun auf den Anfang von Zeile y (0-indexed)
            unsigned char *row = img + y*row_size;

            // Alle Pixel der aktuellen Bildzeile berechnen.
            for (size_t x = 0; x < width; x++) {

                // Komplexe Koordinate des aktuellen Pixels.
                const float complex c = start + (((float)x)*res) - (((float)y)*res) * I;

                // Escape-Time des aktuellen Punktes berechnen.
                const unsigned iterations = escapeTime(n, c);

                // wird n zurückgegeben, ist der Punkt innerhalb des Iterationslimits nicht
                // divergiert und wird schwarz dargestellt.
                if (n == iterations) {
                    if (color) {
                        row[x * 3]=0; // Blau
                        row[x *3+1]=0; // Grün
                        row[x *3+2]=0; // Rot
                    }
                    else {
                        row[x] = 0;
                    }
                } else {
                    if (!color) {
                        //Grauton (von Formel aus der Aufgabe)
                        unsigned color_pixel = 255 - 4 * ( iterations % 64 );
                        row[x]= color_pixel;

                    } else {
                        //Bunt
                        RGB_color colorize = getColor(iterations, n, color);
                        row[x * 3]= colorize.b; // Blau
                        row[x *3+1]= colorize.g; // Grün
                        row[x *3+2]= colorize.r; // Rot
                    }
                }
            }
            //restliche Padding-Bytes einer Zeile mit 0 auffüllen
            for (size_t i=width*bytesPerPixel; i<row_size; i++) {
                row[i]=0;
            }
        }
    }

    /*
     * Ist height positiv, wird eine Bottom-Up Bitmap erstellt.
     * Die erste berechnete Bildzeile ist dabei die unterste Bildzeile.
     */
    else{

        // Jede BMP Zeile muss eine durch 4 teilbare Anzahl an Bytes besitzen.
        // Fehlende Bytes werden am Ende jeder Zeile durch Padding ergänzt.
        size_t row_size = ((width * bytesPerPixel+3)/4)*4;

        // Die Zeilen werden so durchlaufen, damit die Koordinaten passend zur Bottom-Up Darstellung erzeugt werden
        for (ssize_t y = height-1; y >= 0; y--) {

            // *row zeigt nun auf den Anfang von Zeile height-1-y (0-indexed)
            unsigned char *row = img + y*row_size;

            // Alle Pixel der aktuellen Bildzeile berechnen.
            for (size_t x = 0; x < width; x++) {

                // Komplexe Koordinate des aktuellen Pixels.
                const float complex c = start + (((float)x)*res) + (((float)y)*res) * I;

                // Escape-Time des aktuellen Punktes berechnen.
                const unsigned iterations = escapeTime(n, c);


                // wird n zurückgegeben, ist der Punkt innerhalb des Iterationslimits nicht
                // divergiert und wird schwarz dargestellt.
                if (n == iterations) {
                    if (color) {
                        row[x * 3]=0; // Blau
                        row[x *3+1]=0; // Grün
                        row[x *3+2]=0; // Rot
                    }
                    else {
                        row[x] = 0;
                    }
                } else {
                    if (!color) {
                        //Grauton (von Formel aus der Aufgabe)
                        unsigned color_pixel = 255 - 4 * ( iterations % 64 );
                        row[x]=color_pixel;

                    } else {
                        //Bunt
                        RGB_color colorize = getColor(iterations, n, color);
                        row[x * 3]= colorize.b; // 255 ganzes Bild Blau
                        row[x *3+1]= colorize.g; // Grün
                        row[x *3+2]= colorize.r; // Rot
                    }
                }
            }

            //restliche Padding Bits einer Zeile auffüllen
            for (size_t i=width*bytesPerPixel; i<row_size; i++) {
                row[i]=0;
            }
        }
    }
}
