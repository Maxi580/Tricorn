
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

void writeBmp(char *filename, size_t width, ssize_t heightsigned, unsigned char *img) {
    size_t height;
    if (heightsigned < 0) {
        height = heightsigned * (-1);
    }
    else {
        height = heightsigned ;
    }

    int32_t bmp_height = (int32_t) (heightsigned);
    size_t row_size = ((width * 3+3)/4)*4;
    size_t pixelArraySize = row_size * height;
    size_t fileSize = 54 + pixelArraySize; //54 für header der bmp datei
    unsigned char header[54] = {0}; //header deklarieren; initialisieren

    //header schreiben
    header[0] = 'B';
    header[1] = 'M';

    header[2] = (unsigned char) (fileSize);
    header[3] = (unsigned char) (fileSize>>8);
    header[4] = (unsigned char) (fileSize>>16);
    header[5] = (unsigned char) (fileSize>>24);

    header[10] = 54; // 54 Bytes Header

    header[14] = 40;

    header[18] = (unsigned char) (width);
    header[19] = (unsigned char) (width >> 8);
    header[20] = (unsigned char) (width >> 16);
    header[21] = (unsigned char) (width >> 24);
    header[22] = (unsigned char) (bmp_height);
    header[23] = (unsigned char) (bmp_height >> 8);
    header[24] = (unsigned char) (bmp_height >> 16);
    header[25] = (unsigned char) (bmp_height >> 24);

    header[26] = 1; //Farbebened
    header[28] = 24; // Bit pro Pixel (3*8)

    header[34] = (unsigned char) (pixelArraySize);
    header[35] = (unsigned char) (pixelArraySize >> 8);
    header[36] = (unsigned char) (pixelArraySize >> 16);
    header[37] = (unsigned char) (pixelArraySize >> 24);

    FILE *f = fopen(filename, "wb");
    if (f == NULL) {return;}
    fwrite(header, 1, 54, f);
    fwrite(img, pixelArraySize, 1, f );

    fclose(f);

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