#include <stdio.h>
#include <complex.h>

unsigned escapeTime(const unsigned n, const float complex c) {
    /*
     * Berechnet die Escapetime des Punktes 'c'.
     * Zurückgegeben wird die erste Iteration, wo |z| > 2 gilt.
     * Divergiert der Punkt innerhalb von n Iterationen nicht, wird n zurückgegeben
     */

    // Es genügt, den quadrierten Betrag mit 4(=2^2) zu vergleichen
    const double squaredEscapeDistance = 4;

    // Startwert der Iteration z0 = 0;
    unsigned i = 0;
    float complex z = 0;

    /*
     * Re(z)^2 und Im(z)^2 werden gespeichert, da sie sowohl für den
     * Divergenztest als auch für die Berechnung der nächsten Iteration
     * benötigt werden.
     */
    float realSquared = 0, imagSquared = 0;

    do {
        // Es muss immer überprüft werden, ob das Limit erreicht ist (n=0 etc).
        if (i >= n) {
            return i;
        }

        /*
         * Nächste Tricorn Iteration:
         *
         * z = conj(z^2) + c
         *
         * mit
         * Re(z^2) = a^2 - b^2
         * Im(conj(z^2)) = -2ab
         */
        z = realSquared - imagSquared - 2 * crealf(z) * cimagf(z) * I;
        z += c;
        i++;

        // Der Betrag von Z0 ist 0 und muss damit nicht überprüft werden, wir überprüfen erst Z1
        realSquared = crealf(z) * crealf(z), imagSquared = cimagf(z) * cimagf(z);
    } while (realSquared + imagSquared <= squaredEscapeDistance);

    return i;
}
