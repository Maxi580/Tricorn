#include <stdio.h>
#include <complex.h>
#include "BasicTestImplementationBitmap.h"
#include "BasicTestImplementationEscapetime.h"
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
