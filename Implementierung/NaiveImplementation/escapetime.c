#include <stdio.h>
#include <complex.h>
#include <immintrin.h>

unsigned escapeTime(const unsigned n, const float complex c) {
    /* Gibt die Anzahl der Iterationen zurück, nach denen der Betrag von Zi größer als 2 ist.
     * Wenn die Zahl erst in der nten Iteration oder nicht konvergiert wird n zurückgegeben
     */
    const double squaredEscapeDistance = 4;
    // ab |z_n|>2 gilt sicher: |z_n+1|>[z_n|
    unsigned i = 0;
    float complex z = 0;

    float realSquared = 0, imagSquared = 0;
    do {
        // Es muss immer überprüft werden, ob das Limit erreicht ist (n=0 etc).
        if (i >= n) {
            return i;
        }

        z = realSquared - imagSquared - 2 * crealf(z) * cimagf(z) * I;
        z += c;
        i++;

        // Der Betrag von Z0 ist 0 und muss damit nicht überprüft werden, wir überprüfen erst Z1
        // a² und b² wird später auch für zn+1 wieder benötigt und deswegen gespeichert.
        realSquared = crealf(z) * crealf(z), imagSquared = cimagf(z) * cimagf(z);
    } while (realSquared + imagSquared <= squaredEscapeDistance);

    return i;
}

/*unsigned simdEscapeTime(const unsigned n, const float complex c1, const float complex c2, const float complex c3,
                        const float complex c4) {
    const double squaredEscapeDistance = 4;
    unsigned i = 0;

    __m128 z_real = _mm_setzero_ps();
    __m128 z_imagine = _mm_setzero_ps();
    __m128 c_real = _mm_set_ps(crealf(c1), crealf(c2), crealf(c3), crealf(c4));
    __m128 c_imagine = _mm_set_ps(cimagf(c1), cimagf(c2), cimagf(c3), cimagf(c4));

    while (i < n) {

    }



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
}*/


