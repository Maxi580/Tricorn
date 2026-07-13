#include "bitmap.h"
#include <complex.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int writeBmp(char *filename, size_t width, ssize_t heightsigned, unsigned char *img) {
    // Schreibt die Bitmap Datei, gibt 1 bei Fehler und 0 bei Erfolg zurück
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
    size_t fileSize = 54 + pixelArraySize; // 54 für header der bmp datei
    unsigned char header[54] = {0}; // header deklarieren; initialisieren

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
    if (f == NULL) {
        fprintf(stderr, "Fehler beim erstellen der Bitmap Datei.\n");
        return 1;
    }

    if(fwrite(header, 1, 54, f) != 54) {
        fprintf(stderr, "Fehler beim schreiben der Bitmap.\n");
        return 1;
    }

    if(fwrite(img, pixelArraySize, 1, f ) != 1) {
        fprintf(stderr, "Fehler beim schreiben der Bitmap.\n");
        return 1;
    }

    if(fclose(f) != 0) {
        fprintf(stderr, "Fehler beim schließen der Bitmap Datei.\n");
        return 1;
    }

    return 0;
}