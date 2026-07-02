#include <stdio.h>
#include <complex.h>
#include <immintrin.h>

__attribute__((hot))
__m128i simdEscapeTime(const unsigned n, const float complex c1, const float complex c2, const float complex c3,
                        const float complex c4) {
    /* Returns 4 Integer Lanes, mit den Anzahl der Runden nach dem die Koordinate divergiert oder
        falls der Punkt nie divergiert das Limit */

    /* Da wir zn nie berechnen, wurde die Distanzberechnung und Inkrementierung aus der Schleife gezogen,
     * dass bedeutet aber, dass wir das auch für z0 machen, da bei z0 die Distanz 0 ist und der Punkt damit
     * noch nicht divergiert ist, würde allerdings jede lane inkrementiert werden, was allerdings falsch ist.
     * (Man könnte auch n = 1 abfangen, und dann gleich 1er lanes zurückgeben, dass ist aber unclean)*/
    if(n == 0){
    	return _mm_setzero_si128();
    }

    unsigned i = 1;
    __m128i rounds = _mm_setzero_si128();
    __m128i one = _mm_set1_epi32(1);
    __m128 minus_two = _mm_set1_ps(-2.0f);
    __m128 squaredEscapeDistance = _mm_set1_ps(4.0f);

    __m128 z_real = _mm_setzero_ps();;
    __m128 z_imagine = _mm_setzero_ps();
    __m128 c_real = _mm_setr_ps(crealf(c1), crealf(c2), crealf(c3), crealf(c4));
    __m128 c_imagine = _mm_setr_ps(cimagf(c1), cimagf(c2), cimagf(c3), cimagf(c4));

    __m128 realSquared = _mm_setzero_ps(), imagSquared = _mm_setzero_ps();
	/* Zn muss nie berechnet werden, da z(n-1) entweder divergiert dann (n - 1) und sonst immer n zurückgegeben wird
	 * egal ob zn divergiert oder nicht. Deswegen läuft die schleife nur n -1 mal bis zur berechnung von zn-1 (oder
	 * bis alles divergiert), da i = 1 ist.
	 */
    while (i++ < n) {
        // Rounds inkrementieren, falls z noch nicht divergiert (also Betrag <= 4 ist)
        __m128 distance = _mm_add_ps(realSquared, imagSquared);
        __m128 mask = _mm_cmple_ps(distance, squaredEscapeDistance); // FFFFFFFF wo distance <= 4, 0 sonst
        if (_mm_movemask_ps(mask) == 0) break; // Wenn die Maske 0 ist schon alles escaped und man kann aufhören.
        // Setzt 1nsen genau da, wo die maske nicht 0 ist (Castet mask floats als int)
        __m128i inc  = _mm_and_si128(one, _mm_castps_si128(mask));
        rounds  = _mm_add_epi32(rounds, inc);

        // Zi+1 berechnen: z = realSquared - imagSquared - 2 * crealf(z) * cimagf(z) * I;
        z_imagine = _mm_add_ps(c_imagine, _mm_mul_ps(minus_two, _mm_mul_ps(z_real, z_imagine)));
        z_real = _mm_add_ps(c_real, _mm_sub_ps(realSquared, imagSquared));

        // a², b² für Betrag von Zi+1 und zur Berechnung von Zi+2
        realSquared = _mm_mul_ps(z_real, z_real), imagSquared = _mm_mul_ps(z_imagine, z_imagine);
    }

    // Es wird das inkrementiert, was im vorletzten Schritt noch nicht divergiert ist
    __m128 distance = _mm_add_ps(realSquared, imagSquared);
    __m128 mask = _mm_cmple_ps(distance, squaredEscapeDistance);
    __m128i inc  = _mm_and_si128(one, _mm_castps_si128(mask));
    rounds  = _mm_add_epi32(rounds, inc);

    return rounds;
}
