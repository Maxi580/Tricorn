
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

void genImageSIMD(float complex start, size_t width, ssize_t height, float res, unsigned n, bool color, unsigned char* img) {
    const unsigned colorPallete[5][3] = {
        {0,   0,   0},
        {100, 0,   0},
        {255, 50,  0},
        {255, 255, 0},
        {255, 255, 255},
    };
    const unsigned colorPalleteSize = sizeof(colorPallete) / sizeof(colorPallete[0]);
    size_t row_size = ((width * 3+3)/4)*4; // Größe einer Reihe würde auf 4 Byte aufgefüllt(Padding)

    /* Wenn heigt kleiner 0 ist, startet man unten links und geht nach oben also addiert man +y und sonst
     * startet man oben links un zieht y ab, das vorzeichen wird über factor kontrolliert*/
    float factor = height < 0 ? -1.0f : 1.0f;
    ssize_t top = height < 0 ? -height : height;

    /* Width muss nicht durch 4 teilbar sein. Deswegen machen wir 4 koordinaten in parallel so lange das geht und
     * dann am Ende den Rest (also 0 - 3 Elemente) sequenziell*/
    size_t endIndex = width - (width % 4);
    for (ssize_t y = 0; y < top; y++) {
        unsigned char *row = img + y*row_size; // *row zeigt nun auf den Anfang von Zeile y (0-indexed)
        for (size_t x = 0; x < endIndex; x += 4) {
            unsigned iterations[4];
            const float complex c1 = start + (float) x * res + factor * (float) y * res * I;
            const float complex c2 = start + ((float) x + 1) * res + factor * (float) y * res * I;
            const float complex c3 = start + ((float) x + 2) * res + factor * (float) y * res * I;
            const float complex c4 = start + ((float) x + 3) * res + factor * (float) y * res * I;
            _mm_storeu_si128((__m128i*) iterations, simdEscapeTime(n, c1, c2, c3, c4));

            for (int i = 0; i < 4; i++){
                int offset = (x + i) * 3;
                if (n == iterations[i]) { //did not converge
                    //Schwarz (Innere Farbe)
                    row[offset]=0; // Blau
                    row[offset + 1]=0; // Grün
                    row[offset + 2]=0; // Rot
                } else {
                    if (!color) {
                        //Grauton (von Formel aus der Aufgabe)
                        unsigned colorPixel = 255 - 4 * ( iterations[i] % 64 );
                        row[offset]=colorPixel; // Blau
                        row[offset + 1]=colorPixel; // Grün
                        row[offset + 2]=colorPixel; // Rot
                    } else {
                        //Bunt
                        unsigned colorIndex = iterations[i] % colorPalleteSize;
                        row[offset]= colorPallete[colorIndex][0]; // Blau
                        row[offset + 1]= colorPallete[colorIndex][1]; // Grün
                        row[offset + 2]= colorPallete[colorIndex][2]; // Rot
                    }
                }
            }
        }
        // Den Rest von width auch noch fillen (bis zu 3 elemente)
        // const float complex c = start + (((float)x)*res) + (((float)y)*res) * I;
        for (size_t x = endIndex; x < width; x++) {
            const float complex c = start + (((float)x)*res) + factor * (((float)y)*res) * I;
            const unsigned iterations = singleEscapeTime(n, c);

            int offset = x * 3;
            if (n == iterations) {
                row[offset]=0; // Blau
                row[offset+1]=0; // Grün
                row[offset+2]=0; // Rot
            } else {
                if (!color) {
                    unsigned colorPixel = 255 - 4 * ( iterations % 64 );
                    row[offset]=colorPixel; // 255 ganzes Bild Blau
                    row[offset+1]=colorPixel; // Grün
                    row[offset+2]=colorPixel; // Rot
                } else {
                    unsigned farbIndex = iterations % colorPalleteSize;
                    row[offset]= colorPallete[farbIndex][0]; // 255 ganzes Bild Blau
                    row[offset+1]= colorPallete[farbIndex][1]; // Grün
                    row[offset+2]= colorPallete[farbIndex][2]; // Rot
                }
            }
        }

        for (size_t i= width*3; i<row_size; i++) {
            row[i]=0; //restliche Padding Bits einer Zeile auffüllen
        }
    }
}

void writeBmpSIMD(char *filename, size_t width, ssize_t heightsigned, unsigned char *img) {
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

    header[14] = 40; //40-Bit Informationsblock

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