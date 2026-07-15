#include <stdio.h>
#include <complex.h>
#include <immintrin.h>

__attribute__((hot))
__m128i symEscapeTime(const unsigned n, const float complex c1, const float complex c2, const float complex c3,
                        const float complex c4) {
    /*
     * Berechnet die Escape-Time für 4 komplexe Punkte gleichzeitig mit SIMD.
     * Jede Lane erhält am Ende die Iterationszahl, bei der der entsprechende Punkt divergiert.
     * Divergiert ein Punkt in n Iteration nicht, wird n zurückgegeben.
     */

	/*
	 * Sonderfälle:
	 * z0 divergiert nie. Deshalb:
	 * - n = 0: 0
	 * - n = 1: 1
	 */
    if(n <= 1){
    	if (n == 1 ) {
    		return _mm_set1_epi32(1);
    	}
    	return _mm_setzero_si128();
    }

	/*
	 * Die erste Iteration (Berechnung von z1) wird außerhalb der Schleife durchgeführt.
	 * Deshalb beginnt der Schleifenzähler 'i' bei 2 und die Rundenzähler bei 1.
	 */
    unsigned i = 2;

	// Jede Lane enthält die bisherige Escapetime (1).
    __m128i rounds = _mm_set1_epi32(1);

	// Konstanten für die SIMD-Berechnungen
    __m128i one = _mm_set1_epi32(1);
    __m128 minus_two = _mm_set1_ps(-2.0f);
    __m128 squaredEscapeDistance = _mm_set1_ps(4.0f);

	// Anfangswert: z0 = 0
    __m128 z_real = _mm_setzero_ps();;
    __m128 z_imagine = _mm_setzero_ps();

	// Real und Imaginärteil der vier Eingabepunkte (c1 - c4) werden jeweils in eine SIMD Lane geladen
    __m128 c_real = _mm_setr_ps(crealf(c1), crealf(c2), crealf(c3), crealf(c4));
    __m128 c_imagine = _mm_setr_ps(cimagf(c1), cimagf(c2), cimagf(c3), cimagf(c4));

	// Zwischenspeicher für Re(z)^2 und Im(z)^2
    __m128 realSquared = _mm_setzero_ps(), imagSquared = _mm_setzero_ps();
	/*
	 * Erste Iteration:
	 *
	 * z1 = conj(z0^2) + c = c
	 *
	 * -> Dadurch entfällt der Divergenztest für z0, da |z0| = 0 ist.
	 */
	z_imagine = _mm_add_ps(c_imagine, _mm_mul_ps(minus_two, _mm_mul_ps(z_real, z_imagine)));
	z_real = _mm_add_ps(c_real, _mm_sub_ps(realSquared, imagSquared));
	realSquared = _mm_mul_ps(z_real, z_real), imagSquared = _mm_mul_ps(z_imagine, z_imagine);

	// Es wird nur bis z(n-1) berechnet, da es für den Rückgabewert irrelevant ist, ob z(n) noch divergiert.
    while (i++ < n) {
        // (Betrag)^2 der aktuellen vier Punkte berechnen
        __m128 distance = _mm_add_ps(realSquared, imagSquared);

    	/*
    	 * Maske:
    	 *
    	 * FFFFFFFF wo distance <= 4, also noch innerhalb des Fluchtkreises
    	 * 00000000 sonst, also wenn bereits divergiert
    	 */
        __m128 mask = _mm_cmple_ps(distance, squaredEscapeDistance);

    	// Wenn die Maske 0 ist schon alles escaped und man kann aufhören
        if (_mm_movemask_ps(mask) == 0) break;

        // Setzt Einsen genau da, wo die maske nicht 0 ist (Castet mask floats als int)
        __m128i inc  = _mm_and_si128(one, _mm_castps_si128(mask));

    	// Escape-Time dieser Punkte inkrementieren.
        rounds  = _mm_add_epi32(rounds, inc);

        /*
         * Nächste Tricorn Iteration:
         *
         * z = conj(z^2) + c
         *
         * mit
         *
         * Re(z^2) = a^2 - b^2
         * Im(conj(z^2)) = -2ab
         */
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
