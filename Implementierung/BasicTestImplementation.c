
#include <complex.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
/*  betriebsanleitung für timon
 *  gcc Implementierung/BasicTestImplementation.c -o test
 *  ./test
 *  explorer.exe test.bmp
 *
 *
 *
 */


void genImage(size_t width, size_t height, unsigned char *img) {
    size_t row_size = ((width * 3+3)/4)*4;    //größe einer Reihe würde auf 4 Byte aufgrfüllt(Padding)

    for (size_t y = 0; y < height; y++) {
        unsigned char *row = img + y*row_size; // *row zeigt nun auf den Anfang von Zeile 0 (0-indexed)
        for (size_t x = 0; x < width; x++) {
            row[x * 3]=0;
            row[x *3+1]=255;
            row[x *3+2]=0; //bspw. ganzes Bild rot
        }
        for (size_t i=width*3; i<row_size; i++) {
            row[i]=0; //restliche Paddiung Bits einer Zeile auffüllen
        }
    }
}

void writeBmp(char *filename, size_t width, size_t height, unsigned char *img) {
    size_t row_size = ((width * 3+3)/4)*4;
    size_t pixelArraySize = row_size * height;
    size_t fileSize = 54 + pixelArraySize; //54 für header der bmp datei
    unsigned char header[54] = {0}; //header deklarieren; initialisieren

    //header schreiben
    header[0] = 'B';
    header[1] = 'M';

    header[2] = (unsigned char) fileSize;
    header[3] = (unsigned char) fileSize>>8;
    header[4] = (unsigned char) fileSize>>16;
    header[5] = (unsigned char) fileSize>>24;

    header[10] = 54; // 54 Bytes Header

    header[14] = 40;

    header[18] = (unsigned char) width;
    header[19] = (unsigned char) width >> 8;
    header[20] = (unsigned char) width >> 16;
    header[21] = (unsigned char) width >> 24;
    header[22] = (unsigned char) height;
    header[23] = (unsigned char) height >> 8;
    header[24] = (unsigned char) height >> 16;
    header[25] = (unsigned char) height >> 24;

    header[26] = 1; //Farbebened
    header[28] = 24; // Bit pro Pixel (3*8)

    header[34] = (unsigned char) pixelArraySize;
    header[35] = (unsigned char) pixelArraySize >> 8;
    header[36] = (unsigned char) pixelArraySize >> 16;
    header[37] = (unsigned char) pixelArraySize >> 24;

    FILE *f = fopen(filename, "wb");
    if (f == NULL) {return;}
    fwrite(header, 1, 54, f);
    fwrite(img, pixelArraySize, 1, f );

    fclose(f);

}

//int main(void) {
//    size_t width, height;
//    width = 500;
//    height = 300;
//
//    size_t row_size = ((width * 3+3)/4)*4;
//    size_t pixelArraySize = row_size * height;
//
//    unsigned char *img = malloc(pixelArraySize);
//    if (img == NULL) {return 1;} //Fehler
//
//    genImage(width, height, img);
//    writeBmp("test.bmp", width, height, img);
//    free(img);
//    return 0;
//}
