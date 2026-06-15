#include <stdio.h>
#include <complex.h>
#include "bitmap.h"
#include "escapetime.h"
#include <complex.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

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

int main(void) {
    float complex start = 2 + 2*I; //start parameter
    size_t width = 2000; //breite eingeben
    ssize_t heightsigned = 2000; //höhe eingeben
    float res = 4.0f/(float)width; //resolution (auch frei wählbar)
    unsigned n = 500; //Iteration limit
    bool color = false; //false = Grauton berechnen durch Formel / true = Farbpalette


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

    genImage(start, width, heightsigned, res, n, color, img);
    writeBmp("test.bmp", width, heightsigned, img);
    free(img);
    return 0;
}

/* Betriebsanleitung:
 * gcc Implementierung/NaiveImplementation/main.c Implementierung/NaiveImplementation/bitmap.c Implementierung/NaiveImplementation/escapetime.c -o test
 * ./test
 * explorer.exe test.bmp
 */