#include <stdio.h>
#include <complex.h>

unsigned escapeTime(const unsigned n, const float complex c) {
    const double squaredEscapeDistance = 4;
    // ab |z_n|>2 gilt sicher: |z_n+1|>[z_n|
    unsigned i = 0;
    float complex z = 0;

    float realSquared = crealf(z) * crealf(z), imagSquared = cimagf(z) * cimagf(z);
    while (i < n && realSquared + imagSquared < squaredEscapeDistance) {
        // conj(start) = conj(a-bi) = a+bi => conj(a-bi) * conj(a-bi) = a² - b² - 2abi
        z = realSquared - imagSquared - 2 * crealf(z) * cimag(z) * I;
        z += c;

        realSquared = crealf(z) * crealf(z), imagSquared = cimagf(z) * cimagf(z);

        i++;
    }
    return i;
}

int main() {
    const unsigned limit = 10;
    const float complex exampleC = 0.0 + 0.0 * I;


    const unsigned result = escapeTime(limit, exampleC);
    if (limit == result) {
        // besser:? "Did not escape in %u iterations\n", limit
        printf("Did not converge :(");
    } else {
        //warum kein %u?
        printf("Converged after %d (Limit: %d)", result, limit);
    }

}