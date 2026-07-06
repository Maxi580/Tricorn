
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


void genImage(float complex start, size_t width, ssize_t height, float res, unsigned n, bool color, unsigned char* img) {
    const unsigned farbpalletteSize = 5;
    const unsigned farbpallette[5][3] = {
        {0,   0,   0},
        {100, 0,   0},
        {255, 50,  0},
        {255, 255, 0},
        {255, 255, 255},
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
                        unsigned colorPixel = 255 - 4 * ( iterations % 64 );
                        row[x * 3]=colorPixel; // Blau
                        row[x *3+1]=colorPixel; // Grün
                        row[x *3+2]=colorPixel; // Rot
                    } else {
                        //Bunt
                        unsigned farbIndex = iterations % farbpalletteSize;
                        row[x * 3]= farbpallette[farbIndex][0]; // Blau
                        row[x *3+1]= farbpallette[farbIndex][1]; // Grün
                        row[x *3+2]= farbpallette[farbIndex][2]; // Rot
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
                        unsigned colorPixel = 255 - 4 * ( iterations % 64 );
                        row[x * 3]=colorPixel; // 255 ganzes Bild Blau
                        row[x *3+1]=colorPixel; // Grün
                        row[x *3+2]=colorPixel; // Rot
                    } else {
                        //Bunt
                        unsigned farbIndex = iterations % farbpalletteSize;
                        row[x * 3]= farbpallette[farbIndex][0]; // 255 ganzes Bild Blau
                        row[x *3+1]= farbpallette[farbIndex][1]; // Grün
                        row[x *3+2]= farbpallette[farbIndex][2]; // Rot
                    }
                }
            }
            for (size_t i=width*3; i<row_size; i++) {
                row[i]=0; //restliche Padding Bits einer Zeile auffüllen
            }
        }
    }
}

/*
int main(void) {
    size_t width;
    width = 500; //breite eingeben
    ssize_t heightsigned = 700; //höhe eingeben
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
    if (img == NULL) {return 1;} //Fehler

    genImage(width, heightsigned, img);
    writeBmp("test.bmp", width, heightsigned, img);
    free(img);
    return 0;
}
*/