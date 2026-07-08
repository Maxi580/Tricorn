#include <stdio.h>
#include <stdbool.h>
#include <complex.h>
#include <immintrin.h>

#include "escapetime.h"

__attribute__((hot))
twoIntegerLanes  escapeTimeLatencyThroughputTwo(const unsigned n, const float complex a1, const float complex a2, const float complex a3,
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
    const __m128i one = _mm_set1_epi32(1);
    const __m128 circle_radius = _mm_set1_ps(0.0625f); // Sqrt(0.25) = 0.0625
    const __m128 minus_two = _mm_set1_ps(-2.0f);
    const __m128 squared_escape_distance = _mm_set1_ps(4.0f);

	/* Der Throughput der genutzten Operationen ist höher als die Latency, anstatt (wegen der Dependency)
	 * auf das Ergebniss zu warten berechnen wir einfach das nächste*/
	twoFloatLanes z_real = (twoFloatLanes) {_mm_setzero_ps(), _mm_setzero_ps()};
	twoFloatLanes z_imagine = (twoFloatLanes) {_mm_setzero_ps(), _mm_setzero_ps()};
    const twoFloatLanes c_real = (twoFloatLanes) {_mm_setr_ps(crealf(a1), crealf(a2), crealf(a3), crealf(a4)),
		_mm_setr_ps(crealf(b1), crealf(b2), crealf(b3), crealf(b4))};
    const twoFloatLanes c_imagine = (twoFloatLanes) {_mm_setr_ps(cimagf(a1), cimagf(a2), cimagf(a3), cimagf(a4)),
		_mm_setr_ps(cimagf(b1), cimagf(b2), cimagf(b3), cimagf(b4))};

    twoFloatLanes real_squared = (twoFloatLanes) {_mm_setzero_ps(), _mm_setzero_ps()};
	twoFloatLanes imag_squared = (twoFloatLanes) {_mm_setzero_ps(), _mm_setzero_ps()};

	// Circle Check, für beide Punkte checken ob sie im Kreis liegen
	__m128 a_amount = _mm_add_ps(_mm_mul_ps(c_real.a, c_real.a), _mm_mul_ps(c_imagine.a, c_imagine.a));
	__m128 b_amount = _mm_add_ps(_mm_mul_ps(c_real.b, c_real.b), _mm_mul_ps(c_imagine.b, c_imagine.b));
	__m128 radius_mask_a = _mm_cmpgt_ps(a_amount, circle_radius); // FFFFFFFF wo cAmount > sqrt(radius), 0 sonst
	__m128 radius_mask_b = _mm_cmpgt_ps(b_amount, circle_radius);
	if(_mm_movemask_ps(radius_mask_b) == 0 && _mm_movemask_ps(radius_mask_a) == 0) return (twoIntegerLanes)
		{_mm_set1_epi32(n), _mm_set1_epi32(n)};
	bool lane_a_is_finished = false;
	bool lane_b_is_finished = false;

	// Z1 Berechnen, weil man nicht checken muss ob Z0 konvergiert
	z_imagine.a = _mm_add_ps(c_imagine.a, _mm_mul_ps(minus_two, _mm_mul_ps(z_real.a, z_imagine.a)));
	z_imagine.b = _mm_add_ps(c_imagine.b, _mm_mul_ps(minus_two, _mm_mul_ps(z_real.b, z_imagine.b)));

	z_real.a = _mm_add_ps(c_real.a, _mm_sub_ps(real_squared.a, imag_squared.a));
	z_real.b = _mm_add_ps(c_real.b, _mm_sub_ps(real_squared.b, imag_squared.b));

	real_squared.a = _mm_mul_ps(z_real.a, z_real.a);
	real_squared.b = _mm_mul_ps(z_real.b, z_real.b);

	imag_squared.a = _mm_mul_ps(z_imagine.a, z_imagine.a);
	imag_squared.b = _mm_mul_ps(z_imagine.b, z_imagine.b);
	// Zn muss nie berechnet werden, da entweder Zn-1 divergiert oder n zurückgegeben wird => 1 Iteration weniger
	while (i++ < n) {
        // Rounds inkrementieren, falls z noch nicht divergiert (also Betrag <= 4 ist)
    	__m128 distance_a = _mm_add_ps(real_squared.a, imag_squared.a);
    	__m128 distance_b = _mm_add_ps(real_squared.b, imag_squared.b);
        __m128 maskA = _mm_cmple_ps(distance_a, squared_escape_distance); // FFFFFFFF wo distance <= 4, 0 sonst
    	__m128 mask_b = _mm_cmple_ps(distance_b, squared_escape_distance);
        if (_mm_movemask_ps(maskA) == 0) lane_a_is_finished = true; // Wenn die Maske 0 ist schon alles escaped und man kann aufhören.
    	if (_mm_movemask_ps(mask_b) == 0) lane_b_is_finished = true;
    	if(lane_a_is_finished && lane_b_is_finished) break;

        // Setzt 1nsen genau da, wo die maske nicht 0 ist (Castet mask floats als int)
        __m128i incA  = _mm_and_si128(one, _mm_castps_si128(maskA));
    	__m128i incB  = _mm_and_si128(one, _mm_castps_si128(mask_b));
        rounds.a  = _mm_add_epi32(rounds.a , incA);
    	rounds.b  = _mm_add_epi32(rounds.b , incB);

        // Zi+1 berechnen: z = realSquared - imagSquared - 2 * crealf(z) * cimagf(z) * I;
        z_imagine.a = _mm_add_ps(c_imagine.a, _mm_mul_ps(minus_two, _mm_mul_ps(z_real.a, z_imagine.a)));
    	z_imagine.b = _mm_add_ps(c_imagine.b, _mm_mul_ps(minus_two, _mm_mul_ps(z_real.b, z_imagine.b)));

        z_real.a = _mm_add_ps(c_real.a, _mm_sub_ps(real_squared.a, imag_squared.a));
    	z_real.b = _mm_add_ps(c_real.b, _mm_sub_ps(real_squared.b, imag_squared.b));

        // a², b² für Betrag von Zi+1 und zur Berechnung von Zi+2
        real_squared.a = _mm_mul_ps(z_real.a, z_real.a);
    	real_squared.b = _mm_mul_ps(z_real.b, z_real.b);

    	imag_squared.a = _mm_mul_ps(z_imagine.a, z_imagine.a);
    	imag_squared.b = _mm_mul_ps(z_imagine.b, z_imagine.b);
    }

    // Es wird das inkrementiert, was im vorletzten Schritt noch nicht divergiert ist
	__m128 distance_a = _mm_add_ps(real_squared.a, imag_squared.a);
	__m128 distance_b = _mm_add_ps(real_squared.b, imag_squared.b);
	__m128 mask_a = _mm_cmple_ps(distance_a, squared_escape_distance); // FFFFFFFF wo distance <= 4, 0 sonst
	__m128 mask_b = _mm_cmple_ps(distance_b, squared_escape_distance);
	__m128i inc_a = _mm_and_si128(one, _mm_castps_si128(mask_a));
	__m128i inc_b = _mm_and_si128(one, _mm_castps_si128(mask_b));
	rounds.a = _mm_add_epi32(rounds.a , inc_a);
	rounds.b = _mm_add_epi32(rounds.b , inc_b);
    return rounds;
}


__attribute__((hot))
fourIntegerLanes  escapeTimeLatencyThroughputFour(const unsigned n, const float complex a1, const float complex a2, const float complex a3,
	const float complex a4, const float complex b1, const float complex b2, const float complex b3,
	const float complex b4, const float complex c1, const float complex c2, const float complex c3,
	const float complex c4, const float complex d1, const float complex d2, const float complex d3,
	const float complex d4) {
    /* Returned 4 mal 4 Integer Lanes, mit den Anzahl der Runden nach dem die Koordinate divergiert oder
        falls der Punkt nie divergiert das Limit */

    // Bei Z0 können wir direkt 0 und bei Z1 direkt 1 zurückgegeben (weil Z0 nie divergiert)
    if(n <= 1){
    	if (n == 1 ) {
    		return (fourIntegerLanes) {_mm_set1_epi32(1), _mm_set1_epi32(1), _mm_set1_epi32(1),
    			_mm_set1_epi32(1)};
    	}
    	return (fourIntegerLanes) {_mm_setzero_si128(), _mm_setzero_si128(), _mm_setzero_si128(),
    		_mm_setzero_si128()};
    }

	/* i ist 2 (und nicht 0), weil wir Z1 außerhalb berechnen (um die Konvergenz von Z0 nicht zu checken)
	 * und wir Zn nicht berechnen. Dadurch haben wir exakt zwei schleifendurhcläufe weniger.*/
    unsigned i = 2;
	// Rounds startet bei 1, da Z1 und Z0 bereits abgefangen worden sind. Jetzt wird für n>1 geprüft ob Z1 konvergiert.
    fourIntegerLanes rounds = (fourIntegerLanes) {_mm_set1_epi32(1), _mm_set1_epi32(1),
    	_mm_set1_epi32(1), _mm_set1_epi32(1)};

    const __m128i one = _mm_set1_epi32(1);
	const __m128 circle_radius = _mm_set1_ps(0.0625f); //Sqrt(0.25) = 0.0625
    const __m128 minus_two = _mm_set1_ps(-2.0f);
    const __m128 squared_escape_distance = _mm_set1_ps(4.0f);

	/* Der Throughput der genutzten Operationen ist höher als die Latency, anstatt (wegen der Dependency)
	 * auf das Ergebniss zu warten berechnen wir einfach das nächste*/
	fourFloatLanes z_real = (fourFloatLanes) {_mm_setzero_ps(), _mm_setzero_ps(), _mm_setzero_ps(),
		_mm_setzero_ps(),};
	fourFloatLanes z_imagine = (fourFloatLanes) {_mm_setzero_ps(), _mm_setzero_ps(), _mm_setzero_ps(),
		_mm_setzero_ps()};
	fourFloatLanes c_real = (fourFloatLanes) {_mm_setr_ps(crealf(a1), crealf(a2), crealf(a3), crealf(a4)),
		_mm_setr_ps(crealf(b1), crealf(b2), crealf(b3), crealf(b4)), _mm_setr_ps(crealf(c1),
		crealf(c2), crealf(c3), crealf(c4)), _mm_setr_ps(crealf(d1), crealf(d2), crealf(d3),
		crealf(d4))};
	fourFloatLanes c_imagine = (fourFloatLanes) {_mm_setr_ps(cimagf(a1), cimagf(a2), cimagf(a3), cimagf(a4)),
		_mm_setr_ps(cimagf(b1), cimagf(b2), cimagf(b3), cimagf(b4)), _mm_setr_ps(cimagf(c1),
		cimagf(c2), cimagf(c3), cimagf(c4)), _mm_setr_ps(cimagf(d1), cimagf(d2), cimagf(d3),
		cimagf(d4))};

	fourFloatLanes real_squared = (fourFloatLanes) {_mm_setzero_ps(), _mm_setzero_ps(), _mm_setzero_ps(),
			_mm_setzero_ps(),};
	fourFloatLanes imag_squared = (fourFloatLanes) {_mm_setzero_ps(), _mm_setzero_ps(), _mm_setzero_ps(),
			_mm_setzero_ps(),};

	// Circle Check, für beide Punkte checken ob sie im Kreis liegen
	__m128 a_amount = _mm_add_ps(_mm_mul_ps(c_real.a, c_real.a), _mm_mul_ps(c_imagine.a, c_imagine.a));
	__m128 b_amount = _mm_add_ps(_mm_mul_ps(c_real.b, c_real.b), _mm_mul_ps(c_imagine.b, c_imagine.b));
	__m128 c_amount = _mm_add_ps(_mm_mul_ps(c_real.c, c_real.c), _mm_mul_ps(c_imagine.c, c_imagine.c));
	__m128 d_amount = _mm_add_ps(_mm_mul_ps(c_real.d, c_real.d), _mm_mul_ps(c_imagine.d, c_imagine.d));
	__m128 radius_mask_a = _mm_cmpgt_ps(a_amount, circle_radius); // FFFFFFFF wo cAmount > sqrt(radius), 0 sonst
	__m128 radius_mask_b = _mm_cmpgt_ps(b_amount, circle_radius);
	__m128 radius_mask_c = _mm_cmpgt_ps(c_amount, circle_radius);
	__m128 radius_mask_d = _mm_cmpgt_ps(d_amount, circle_radius);
	bool lane_a_is_finished = _mm_movemask_ps(radius_mask_a) == 0 ? true : false;
	bool lane_b_is_finished = _mm_movemask_ps(radius_mask_b) == 0 ? true : false;
	bool lane_c_is_finished = _mm_movemask_ps(radius_mask_c) == 0 ? true : false;
	bool lane_d_is_finished = _mm_movemask_ps(radius_mask_d) == 0 ? true : false;
	if(lane_a_is_finished && lane_b_is_finished && lane_c_is_finished && lane_d_is_finished) return
		(fourIntegerLanes) {_mm_set1_epi32(n), _mm_set1_epi32(n), _mm_set1_epi32(n), _mm_set1_epi32(n)};

	// Z1 Berechnen, weil man nicht checken muss ob Z0 konvergiert
	z_imagine.a = _mm_add_ps(c_imagine.a, _mm_mul_ps(minus_two, _mm_mul_ps(z_real.a, z_imagine.a)));
	z_imagine.b = _mm_add_ps(c_imagine.b, _mm_mul_ps(minus_two, _mm_mul_ps(z_real.b, z_imagine.b)));
	z_imagine.c = _mm_add_ps(c_imagine.c, _mm_mul_ps(minus_two, _mm_mul_ps(z_real.c, z_imagine.c)));
	z_imagine.d = _mm_add_ps(c_imagine.d, _mm_mul_ps(minus_two, _mm_mul_ps(z_real.d, z_imagine.d)));

	z_real.a = _mm_add_ps(c_real.a, _mm_sub_ps(real_squared.a, imag_squared.a));
	z_real.b = _mm_add_ps(c_real.b, _mm_sub_ps(real_squared.b, imag_squared.b));
	z_real.c = _mm_add_ps(c_real.c, _mm_sub_ps(real_squared.c, imag_squared.c));
	z_real.d = _mm_add_ps(c_real.d, _mm_sub_ps(real_squared.d, imag_squared.d));

	real_squared.a = _mm_mul_ps(z_real.a, z_real.a);
	real_squared.b = _mm_mul_ps(z_real.b, z_real.b);
	real_squared.c = _mm_mul_ps(z_real.c, z_real.c);
	real_squared.d = _mm_mul_ps(z_real.d, z_real.d);

	imag_squared.a = _mm_mul_ps(z_imagine.a, z_imagine.a);
	imag_squared.b = _mm_mul_ps(z_imagine.b, z_imagine.b);
	imag_squared.c = _mm_mul_ps(z_imagine.c, z_imagine.c);
	imag_squared.d = _mm_mul_ps(z_imagine.d, z_imagine.d);
	// Zn muss nie berechnet werden, da entweder Zn-1 divergiert oder n zurückgegeben wird => 1 Iteration weniger
	while (i++ < n) {
        // Rounds inkrementieren, falls z noch nicht divergiert (also Betrag <= 4 ist)
    	__m128 distance_a = _mm_add_ps(real_squared.a, imag_squared.a);
    	__m128 distance_b = _mm_add_ps(real_squared.b, imag_squared.b);
		__m128 distance_c = _mm_add_ps(real_squared.c, imag_squared.c);
		__m128 distance_d = _mm_add_ps(real_squared.d, imag_squared.d);
        __m128 mask_a = _mm_cmple_ps(distance_a, squared_escape_distance); // FFFFFFFF wo distance <= 4, 0 sonst
    	__m128 mask_b = _mm_cmple_ps(distance_b, squared_escape_distance);
		__m128 mask_c = _mm_cmple_ps(distance_c, squared_escape_distance);
		__m128 mask_d = _mm_cmple_ps(distance_d, squared_escape_distance);
        if (_mm_movemask_ps(mask_a) == 0) lane_a_is_finished = true; // Wenn die Maske 0 ist schon alles escaped und man kann aufhören.
    	if (_mm_movemask_ps(mask_b) == 0) lane_b_is_finished = true;
		if (_mm_movemask_ps(mask_c) == 0) lane_c_is_finished = true;
		if (_mm_movemask_ps(mask_d) == 0) lane_d_is_finished = true;
    	if (lane_a_is_finished && lane_b_is_finished && lane_c_is_finished && lane_d_is_finished)break;

        // Setzt 1nsen genau da, wo die maske nicht 0 ist (Castet mask floats als int)
        __m128i inc_a  = _mm_and_si128(one, _mm_castps_si128(mask_a));
    	__m128i inc_b  = _mm_and_si128(one, _mm_castps_si128(mask_b));
		__m128i inc_c  = _mm_and_si128(one, _mm_castps_si128(mask_c));
		__m128i inc_d  = _mm_and_si128(one, _mm_castps_si128(mask_d));
        rounds.a = _mm_add_epi32(rounds.a , inc_a);
    	rounds.b = _mm_add_epi32(rounds.b , inc_b);
		rounds.c = _mm_add_epi32(rounds.c , inc_c);
		rounds.d = _mm_add_epi32(rounds.d , inc_d);

        // Zi+1 berechnen: z = realSquared - imagSquared - 2 * crealf(z) * cimagf(z) * I;
        z_imagine.a = _mm_add_ps(c_imagine.a, _mm_mul_ps(minus_two, _mm_mul_ps(z_real.a, z_imagine.a)));
    	z_imagine.b = _mm_add_ps(c_imagine.b, _mm_mul_ps(minus_two, _mm_mul_ps(z_real.b, z_imagine.b)));
		z_imagine.c = _mm_add_ps(c_imagine.c, _mm_mul_ps(minus_two, _mm_mul_ps(z_real.c, z_imagine.c)));
		z_imagine.d = _mm_add_ps(c_imagine.d, _mm_mul_ps(minus_two, _mm_mul_ps(z_real.d, z_imagine.d)));

        z_real.a = _mm_add_ps(c_real.a, _mm_sub_ps(real_squared.a, imag_squared.a));
    	z_real.b = _mm_add_ps(c_real.b, _mm_sub_ps(real_squared.b, imag_squared.b));
		z_real.c = _mm_add_ps(c_real.c, _mm_sub_ps(real_squared.c, imag_squared.c));
		z_real.d = _mm_add_ps(c_real.d, _mm_sub_ps(real_squared.d, imag_squared.d));

        // a², b² für Betrag von Zi+1 und zur Berechnung von Zi+2
        real_squared.a = _mm_mul_ps(z_real.a, z_real.a);
    	real_squared.b = _mm_mul_ps(z_real.b, z_real.b);
		real_squared.c = _mm_mul_ps(z_real.c, z_real.c);
		real_squared.d = _mm_mul_ps(z_real.d, z_real.d);

    	imag_squared.a = _mm_mul_ps(z_imagine.a, z_imagine.a);
    	imag_squared.b = _mm_mul_ps(z_imagine.b, z_imagine.b);
		imag_squared.c = _mm_mul_ps(z_imagine.c, z_imagine.c);
		imag_squared.d = _mm_mul_ps(z_imagine.d, z_imagine.d);
    }

    // Es wird das inkrementiert, was im vorletzten Schritt noch nicht divergiert ist
	__m128 distance_a = _mm_add_ps(real_squared.a, imag_squared.a);
	__m128 distance_b = _mm_add_ps(real_squared.b, imag_squared.b);
	__m128 distance_c = _mm_add_ps(real_squared.c, imag_squared.c);
	__m128 distance_d = _mm_add_ps(real_squared.d, imag_squared.d);
	__m128 mask_a = _mm_cmple_ps(distance_a, squared_escape_distance); // FFFFFFFF wo distance <= 4, 0 sonst
	__m128 mask_b = _mm_cmple_ps(distance_b, squared_escape_distance);
	__m128 mask_c = _mm_cmple_ps(distance_c, squared_escape_distance);
	__m128 mask_d = _mm_cmple_ps(distance_d, squared_escape_distance);
	__m128i inc_a  = _mm_and_si128(one, _mm_castps_si128(mask_a));
	__m128i inc_b  = _mm_and_si128(one, _mm_castps_si128(mask_b));
	__m128i inc_c  = _mm_and_si128(one, _mm_castps_si128(mask_c));
	__m128i inc_d  = _mm_and_si128(one, _mm_castps_si128(mask_d));
	rounds.a  = _mm_add_epi32(rounds.a , inc_a);
	rounds.b  = _mm_add_epi32(rounds.b , inc_b);
	rounds.c  = _mm_add_epi32(rounds.c , inc_c);
	rounds.d  = _mm_add_epi32(rounds.d , inc_d);
    return rounds;
}
