#include "bitmap.h"
#include <complex.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

int writeBmp(char *filename, ssize_t width, ssize_t height_signed, unsigned char *img, bool color) {
    // Schreibt die Bitmap Datei, gibt 1 bei Fehler und 0 bei Erfolg zurück
    /* Die range checks sind auch schon in der main, sodass das img nicht unnötig generiert wird, aber sollten
     * trotzdem in der Funktion selbst auch geprüft werden*/
    if (width <= 0 || width > BMP_MAX_RANGE) {
        fprintf(stderr, "Error: Width out of Range ]0, 2.147.483.647]\n");
        return 1;
    }
    if (height_signed > BMP_MAX_RANGE || height_signed < BMP_MIN_RANGE || height_signed == 0) {
        fprintf(stderr, "Error: Height out of Range [-2.147.483.648, 0[ / ] 0, 2.147.483.647]\n");
        return 1;
    }

    size_t height = abs_size(height_signed);

    int32_t bmp_height = (int32_t) (height_signed);
    int bytesPerPixel = color ? 3 : 1;
    size_t row_size = ((width * bytesPerPixel+3)/4)*4;
    size_t pixelArraySize = row_size * height;
    size_t paletteSize = color ? COLOR_PALETTE_SIZE : GREY_PALETTE_SIZE;
    size_t pixelOffset = BMP_HEADER_SIZE + paletteSize;
    size_t fileSize = pixelOffset + pixelArraySize; // 54 für header der bmp datei
    unsigned char header[54] = {0}; // header deklarieren; initialisieren

    if (fileSize > BMP_MAX_FILE_SIZE) {
        fprintf(stderr, "Error: Bitmap Größe out of Range (maximal 4 GB)\n");
        return 1;
    }

    //header schreiben
    header[0] = 'B';
    header[1] = 'M';

    header[2] = (unsigned char) (fileSize);
    header[3] = (unsigned char) (fileSize>>8);
    header[4] = (unsigned char) (fileSize>>16);
    header[5] = (unsigned char) (fileSize>>24);

    header[10] = (unsigned char)(pixelOffset); //header
    header[11] = (unsigned char)(pixelOffset >> 8);
    header[12] = (unsigned char)(pixelOffset >> 16);
    header[13] = (unsigned char)(pixelOffset >> 24);

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
    header[28] = 8 * bytesPerPixel; // Bit pro Pixel (3*8)

    header[34] = (unsigned char) (pixelArraySize);
    header[35] = (unsigned char) (pixelArraySize >> 8);
    header[36] = (unsigned char) (pixelArraySize >> 16);
    header[37] = (unsigned char) (pixelArraySize >> 24);

    if (!color) {
        const uint32_t colorsUsed = 256;
        header[46] = (unsigned char)(colorsUsed);
        header[47] = (unsigned char)(colorsUsed >> 8);
        header[48] = (unsigned char)(colorsUsed >> 16);
        header[49] = (unsigned char)(colorsUsed >> 24);
    }

    FILE *f = fopen(filename, "wb");
    if (f == NULL) {
        fprintf(stderr, "Fehler beim erstellen der Bitmap Datei.\n");
        return 1;
    }

    if(fwrite(header, 1, sizeof(header), f) != sizeof(header)) {
        fprintf(stderr, "Fehler beim schreiben der Bitmap.\n");
        fclose(f);
        return 1;
    }
    if (!color) {
        unsigned char palette[256][4];
        //Farbpalette, damit für jeden Grauton(bspw. 128 <128,128,128,0>)das BGR abgerufen werden kann.
        for (size_t i = 0; i < 256; i++) {
            palette[i][0] = (unsigned char)i;
            palette[i][1] = (unsigned char)i;
            palette[i][2] = (unsigned char)i;
            palette[i][3] = 0;
        }

        if (fwrite(palette, 1, sizeof(palette), f) != sizeof(palette)) {
            fprintf(stderr, "Fehler beim Schreiben der Farbpalette.\n");
            fclose(f);
            return 1;
        }
    }


    if(fwrite(img, pixelArraySize, 1, f ) != pixelArraySize) {
        fprintf(stderr, "Fehler beim schreiben der Bitmap.\n");
        fclose(f);
        return 1;
    }

    if(fclose(f) != 0) {
        fprintf(stderr, "Fehler beim schließen der Bitmap Datei.\n");
        return 1;
    }

    return 0;
}