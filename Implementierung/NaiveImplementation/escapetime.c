#include <stdio.h>
#include <complex.h>

unsigned escapeTime(const unsigned n, const float complex c) {
    /* Gibt die Anzahl der Iterationen zurück, nach denen der Betrag von Zi größer als 2 ist.
     * Wenn die Zahl erst in der nten Iteration oder nicht konvergiert wird n zurückgegeben
     *
     * (Aktuell wird zn noch berechnet, was eigentlich unnötig ist, weil unabhängig vom ergebnis n zurückgegeben wird)
     * 1. Checken ob maximale iteration erreicht ist
     * 2. Erst dann Betrag ausrechnen, ist unnötig wenn iteration eh zu groß ist
     * 3. Falls Betrag kleiner, bereits berechnete Werte wieder verwenden, weil effizienz
     */
    const double squaredEscapeDistance = 4;
    // ab |z_n|>2 gilt sicher: |z_n+1|>[z_n|
    unsigned i = 0;
    float complex z = 0;

    while (i < n) {
        float realSquared = crealf(z) * crealf(z), imagSquared = cimagf(z) * cimagf(z);

        if (realSquared + imagSquared > squaredEscapeDistance) {
            break;
        }

        // conj(start) = conj(a-bi) = a+bi => conj(a-bi) * conj(a-bi) = a² - b² - 2abi
        z = realSquared - imagSquared - 2 * crealf(z) * cimag(z) * I;
        z += c;

        i++;
    }
    return i;
}
/*
int main() {
    const unsigned limit = 10;
    const float complex exampleC = 0.0 + 0.0 * I;


    const unsigned result = escapeTime(limit, exampleC);
    if (limit == result) {
        // besser:? "Did not escape in %u iterations\n", limit
        printf("Did not converge :(");
    } else {
        printf("Converged after %d (Limit: %d)", result, limit);
    }
//
}*/