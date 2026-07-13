
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
/*  betriebsanleitung für timon
 *  gcc Implementierung/BasicTestImplementation.c -o test
 *  ./test
 *  explorer.exe test.bmp
 */


void tricorn_V1(float complex start, size_t width, ssize_t height, float res, unsigned n, bool color, unsigned char* img) {
    const unsigned naive_color_pallete_size = 4;
    const unsigned naive_color_pallete[4][3] = {
        {128, 0, 0},
        {255, 0, 0},
        {255, 100, 0},
        {255, 255, 0},
    };

    if (height < 0) { // TOP-DOWN
        height = height * (-1);
        size_t row_size = ((width * 3+3)/4)*4;    //größe einer Reihe würde auf 4 Byte aufgefüllt(Padding)

        for (ssize_t y = 0; y < height; y++) {
            unsigned char *row = img + y*row_size; // *row zeigt nun auf den Anfang von Zeile y (0-indexed)
            for (size_t x = 0; x < width; x++) {

               // const float cx = ((float)x - (float)width/2) * res;
                //const float cy = ((float)y - (float)height/2) * res;
               // const float complex c = start + cx + cy * I;
                const float complex c = start + (((float)x)*res) - (((float)y)*res) * I;
                const unsigned iterations = escapeTime(n, c);

                if (n == iterations) { //did not converge
                    //Schwarz (Innere Farbe)
                    row[x * 3]=0; // Blau
                    row[x *3+1]=0; // Grün
                    row[x *3+2]=0; // Rot
                } else {
                    if (!color) {
                        //Grauton (von Formel aus der Aufgabe)
                        unsigned color_pixel = 255 - 4 * ( iterations % 64 );
                        row[x * 3]= color_pixel; // Blau
                        row[x *3+1]= color_pixel; // Grün
                        row[x *3+2]= color_pixel; // Rot
                    } else {
                        //Bunt
                        unsigned farbIndex = iterations % naive_color_pallete_size;
                        row[x * 3]= naive_color_pallete[farbIndex][0]; // Blau
                        row[x *3+1]= naive_color_pallete[farbIndex][1]; // Grün
                        row[x *3+2]= naive_color_pallete[farbIndex][2]; // Rot
                    }
                }
            }
            for (size_t i=width*3; i<row_size; i++) {
                row[i]=0; //restliche Padding Bits einer Zeile auffüllen
            }
        }
    }
    else{ // BOTTOM-UP
        size_t row_size = ((width * 3+3)/4)*4;    //größe einer Reihe würde auf 4 Byte aufgefüllt(Padding)


        for (ssize_t y = height-1; y >= 0; y--) {
            unsigned char *row = img + y*row_size; // *row zeigt nun auf den Anfang von Zeile height-1-y (0-indexed)
            for (size_t x = 0; x < width; x++) {

                //const float cx = ((float)x - (float)width/2) * res;
                //const float cy = ((float)y - (float)height/2) * res;
               // const float complex c = start + cx + cy * I;
                const float complex c = start + (((float)x)*res) + (((float)y)*res) * I;
                const unsigned iterations = escapeTime(n, c);

                if (n == iterations) { //did not converge
                    //Schwarz (Innere Farbe)
                    row[x * 3]=0; // Blau
                    row[x *3+1]=0; // Grün
                    row[x *3+2]=0; // Rot
                } else {
                    if (!color) {
                        //Grauton (von Formel aus der Aufgabe)
                        unsigned color_pixel = 255 - 4 * ( iterations % 64 );
                        row[x * 3]=color_pixel; // 255 ganzes Bild Blau
                        row[x *3+1]=color_pixel; // Grün
                        row[x *3+2]=color_pixel; // Rot
                    } else {
                        //Bunt
                        unsigned farbIndex = iterations % naive_color_pallete_size;
                        row[x * 3]= naive_color_pallete[farbIndex][0]; // 255 ganzes Bild Blau
                        row[x *3+1]= naive_color_pallete[farbIndex][1]; // Grün
                        row[x *3+2]= naive_color_pallete[farbIndex][2]; // Rot
                    }
                }
            }
            for (size_t i=width*3; i<row_size; i++) {
                row[i]=0; //restliche Padding Bits einer Zeile auffüllen
            }
        }
    }
}
