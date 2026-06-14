#include <stdio.h>
#include <complex.h>
#include "BasicTestImplementationBitmap.h"
#include "BasicTestImplementationEscapetime.h"


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