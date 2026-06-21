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

/*__m128i simdEscapeTime(const unsigned n, const float complex c1, const float complex c2, const float complex c3,
                        const float complex c4) {
    // Gibt 4 integer lanes mit den Runden, nachdem das entsprechende c konvergiert ist zurück
    // Aktuell wird es n mal durchgeführt und nicht gechecked ob schon was konvergiert ist etc.
    unsigned i = 0;
    __m128i rounds = _mm_setzero_si128();
    __m128i one = _mm_set1_epi32(1);
    __m128 minus_two = _mm_set1_ps(-2.0f);
    __m128 squaredEscapeDistance = _mm_set1_ps(4.0f);

    __m128 z_real = _mm_setzero_ps();;
    __m128 z_imagine = _mm_setzero_ps();
    __m128 c_real = _mm_set_ps(crealf(c1), crealf(c2), crealf(c3), crealf(c4));
    __m128 c_imagine = _mm_set_ps(cimagf(c1), cimagf(c2), cimagf(c3), cimagf(c4));

    __m128 realSquared = _mm_setzero_ps(), imagSquared = _mm_setzero_ps();
    while (i < n) {
        // Inkrementiert rounds, bei den Lanes, in denen Zi noch nicht divergiert ist
        __m128 distance = _mm_add_ps(realSquared, imagSquared);
        __m128 mask = _mm_cmple_ps(distance, squaredEscapeDistance); // 0 wo distance < 4, FFFFF sonst
        // Setzt 1nsen genau da, wo die maske nicht 0 ist (Castet mask floats als int)
        __m128i inc  = _mm_and_si128(one, _mm_castps_si128(mask));
        rounds  = _mm_add_epi32(rounds, inc);

        // Zi+1 berechnen: z = realSquared - imagSquared - 2 * crealf(z) * cimagf(z) * I;
        z_imagine = _mm_add_ps(c_imagine, _mm_mul_ps(minus_two, _mm_mul_ps(z_real, z_imagine)));
        z_real = _mm_add_ps(c_real, _mm_sub_ps(realSquared, imagSquared));

        // a², b² für Betrag von Zi+1 und Zi+2
        realSquared = _mm_mul_ps(z_real, z_real), imagSquared = _mm_mul_ps(z_imagine, z_imagine);

        i++;
    }
    return rounds;
}*/



