#include <stdio.h>
#include <complex.h>

unsigned escapeTime(const unsigned n, float complex start, const float complex c) {
    const double squaredEscapeDistance = 4;
    unsigned i = 0;

    while (i < n && crealf(start * conj(start)) < squaredEscapeDistance) {
        start = conj(start) * conj(start);
        start += c;

        i++;
    }
    return i;
}

int main() {
    const unsigned limit = 10;
    const float complex exampleStart = 0;
    const float complex exampleC = 1.0 + 2.0 * I;


    const unsigned result = escapeTime(limit, exampleStart, exampleC);
    if (limit == result) {
        printf("Did not converge :(");
    } else {
        printf("Converged after %d (Limit: %d)", result, limit);
    }


}