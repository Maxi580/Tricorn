#include <stdio.h>
#include <stdbool.h>
#include <complex.h>
#include <immintrin.h>

#include "escapetime.h"

__attribute__((hot))
twoIntegerLanes  escapeTimeLatencyThroughput(const unsigned n, const float complex a1, const float complex a2, const float complex a3,
	const float complex a4, const float complex b1, const float complex b2, const float complex b3,
	const float complex b4) {
    /* Returned 2 mal 4 Integer Lanes, mit den Anzahl der Runden nach dem die Koordinate divergiert oder
        falls der Punkt nie divergiert das Limit */

    // Bei Z0 können wir direkt 0 und bei Z1 direkt 1 zurückgegeben (weil Z0 nie divergiert)
    if(n <= 1){
    	if (n == 1 ) {
    		return (twoIntegerLanes) {_mm_set1_epi32(1), _mm_set1_epi32(1)};
    	}
    	return (twoIntegerLanes) {_mm_setzero_si128(), _mm_setzero_si128()};
    }

	/* i ist 2 (und nicht 0), weil wir Z1 außerhalb berechnen (um die Konvergenz von Z0 nicht zu checken)
	 * und wir Zn nicht berechnen. Dadurch haben wir exakt zwei schleifendurhcläufe weniger.*/
    unsigned i = 2;
	// Rounds startet bei 1, da Z1 und Z0 bereits abgefangen worden sind. Jetzt wird für n>1 geprüft ob Z1 konvergiert.
    twoIntegerLanes rounds = (twoIntegerLanes) {_mm_set1_epi32(1), _mm_set1_epi32(1)};
    __m128i one = _mm_set1_epi32(1);
	__m128 circleRadius = _mm_set1_ps(0.0625f); //Sqrt(0.25) = 0.0625
    __m128 minus_two = _mm_set1_ps(-2.0f);
    __m128 squaredEscapeDistance = _mm_set1_ps(4.0f);

	/* Der Throughput der genutzten Operationen ist höher als die Latency, anstatt (wegen der Dependency)
	 * auf das Ergebniss zu warten berechnen wir einfach das nächste*/
	twoFloatLanes z_real = (twoFloatLanes) {_mm_setzero_ps(), _mm_setzero_ps()};
	twoFloatLanes z_imagine = (twoFloatLanes) {_mm_setzero_ps(), _mm_setzero_ps()};
	twoFloatLanes c_real = (twoFloatLanes) {_mm_setr_ps(crealf(a1), crealf(a2), crealf(a3), crealf(a4)),
		_mm_setr_ps(crealf(b1), crealf(b2), crealf(b3), crealf(b4))};
	twoFloatLanes c_imagine = (twoFloatLanes) {_mm_setr_ps(cimagf(a1), cimagf(a2), cimagf(a3), cimagf(a4)),
		_mm_setr_ps(cimagf(b1), cimagf(b2), cimagf(b3), cimagf(b4))};

    twoFloatLanes realSquared = (twoFloatLanes) {_mm_setzero_ps(), _mm_setzero_ps()};
	twoFloatLanes imagSquared = (twoFloatLanes) {_mm_setzero_ps(), _mm_setzero_ps()};

	// Circle Check, für beide Punkte checken ob sie im Kreis liegen
	__m128 aAmount = _mm_add_ps(_mm_mul_ps(c_real.a, c_real.a), _mm_mul_ps(c_imagine.a, c_imagine.a));
	__m128 bAmount = _mm_add_ps(_mm_mul_ps(c_real.b, c_real.b), _mm_mul_ps(c_imagine.b, c_imagine.b));
	__m128 radiusMaskA = _mm_cmpgt_ps(aAmount, circleRadius); // FFFFFFFF wo cAmount > sqrt(radius), 0 sonst
	__m128 radiusMaskB = _mm_cmpgt_ps(bAmount, circleRadius);
	bool laneAisFinished = _mm_movemask_ps(radiusMaskA) == 0 ? true : false;
	bool laneBisFinished = _mm_movemask_ps(radiusMaskB) == 0 ? true : false;
	if(laneAisFinished && laneBisFinished) return (twoIntegerLanes) {_mm_set1_epi32(n), _mm_set1_epi32(n)};

	// Z1 Berechnen, weil man nicht checken muss ob Z0 konvergiert
	z_imagine.a = _mm_add_ps(c_imagine.a, _mm_mul_ps(minus_two, _mm_mul_ps(z_real.a, z_imagine.a)));
	z_imagine.b = _mm_add_ps(c_imagine.b, _mm_mul_ps(minus_two, _mm_mul_ps(z_real.b, z_imagine.b)));

	z_real.a = _mm_add_ps(c_real.a, _mm_sub_ps(realSquared.a, imagSquared.a));
	z_real.b = _mm_add_ps(c_real.b, _mm_sub_ps(realSquared.b, imagSquared.b));

	realSquared.a = _mm_mul_ps(z_real.a, z_real.a);
	realSquared.b = _mm_mul_ps(z_real.b, z_real.b);

	imagSquared.a = _mm_mul_ps(z_imagine.a, z_imagine.a);
	imagSquared.b = _mm_mul_ps(z_imagine.b, z_imagine.b);
	// Zn muss nie berechnet werden, da entweder Zn-1 divergiert oder n zurückgegeben wird => 1 Iteration weniger
	while (i++ < n) {
        // Rounds inkrementieren, falls z noch nicht divergiert (also Betrag <= 4 ist)
    	__m128 distanceA = _mm_add_ps(realSquared.a, imagSquared.a);
    	__m128 distanceB = _mm_add_ps(realSquared.b, imagSquared.b);
        __m128 maskA = _mm_cmple_ps(distanceA, squaredEscapeDistance); // FFFFFFFF wo distance <= 4, 0 sonst
    	__m128 maskB = _mm_cmple_ps(distanceB, squaredEscapeDistance);
        if (_mm_movemask_ps(maskA) == 0) laneAisFinished = true; // Wenn die Maske 0 ist schon alles escaped und man kann aufhören.
    	if (_mm_movemask_ps(maskB) == 0) laneBisFinished = true;
    	if(laneAisFinished && laneBisFinished)break;;

        // Setzt 1nsen genau da, wo die maske nicht 0 ist (Castet mask floats als int)
        __m128i incA  = _mm_and_si128(one, _mm_castps_si128(maskA));
    	__m128i incB  = _mm_and_si128(one, _mm_castps_si128(maskB));
        rounds.a  = _mm_add_epi32(rounds.a , incA);
    	rounds.b  = _mm_add_epi32(rounds.b , incB);

        // Zi+1 berechnen: z = realSquared - imagSquared - 2 * crealf(z) * cimagf(z) * I;
        z_imagine.a = _mm_add_ps(c_imagine.a, _mm_mul_ps(minus_two, _mm_mul_ps(z_real.a, z_imagine.a)));
    	z_imagine.b = _mm_add_ps(c_imagine.b, _mm_mul_ps(minus_two, _mm_mul_ps(z_real.b, z_imagine.b)));

        z_real.a = _mm_add_ps(c_real.a, _mm_sub_ps(realSquared.a, imagSquared.a));
    	z_real.b = _mm_add_ps(c_real.b, _mm_sub_ps(realSquared.b, imagSquared.b));

        // a², b² für Betrag von Zi+1 und zur Berechnung von Zi+2
        realSquared.a = _mm_mul_ps(z_real.a, z_real.a);
    	realSquared.b = _mm_mul_ps(z_real.b, z_real.b);

    	imagSquared.a = _mm_mul_ps(z_imagine.a, z_imagine.a);
    	imagSquared.b = _mm_mul_ps(z_imagine.b, z_imagine.b);
    }

    // Es wird das inkrementiert, was im vorletzten Schritt noch nicht divergiert ist
	__m128 distanceA = _mm_add_ps(realSquared.a, imagSquared.a);
	__m128 distanceB = _mm_add_ps(realSquared.b, imagSquared.b);
	__m128 maskA = _mm_cmple_ps(distanceA, squaredEscapeDistance); // FFFFFFFF wo distance <= 4, 0 sonst
	__m128 maskB = _mm_cmple_ps(distanceB, squaredEscapeDistance);
	__m128i incA  = _mm_and_si128(one, _mm_castps_si128(maskA));
	__m128i incB  = _mm_and_si128(one, _mm_castps_si128(maskB));
	rounds.a  = _mm_add_epi32(rounds.a , incA);
	rounds.b  = _mm_add_epi32(rounds.b , incB);
    return rounds;
}
