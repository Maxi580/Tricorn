#include <stdio.h>
#include <complex.h>

unsigned escapeTime(const unsigned n, float complex start, const float complex c) {
    const double squaredEscapeDistance = 4;
    unsigned i = 0;

    float realSquared = crealf(start) * crealf(start), imagSquared = cimagf(start) * cimagf(start);
    while (i < n && realSquared + imagSquared < squaredEscapeDistance) {
        // conj(start) = conj(a-bi) = a+bi => conj(a-bi) * conj(a-bi) = a² - b² - 2abi
        start = realSquared - imagSquared - 2 * crealf(start) * cimag(start) * I;
        start += c;

        realSquared = crealf(start) * crealf(start), imagSquared = cimagf(start) * cimagf(start);

        i++;
    }
    return i;
}

int main() {
    const unsigned limit = 10;
    const float complex exampleStart = 0;
    const float complex exampleC = 0.0 + 0.0 * I;


    const unsigned result = escapeTime(limit, exampleStart, exampleC);
    if (limit == result) {
        printf("Did not converge :(");
    } else {
        printf("Converged after %d (Limit: %d)", result, limit);
    }
}