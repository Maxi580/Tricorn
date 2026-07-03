#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../NaiveImplementation/escapetime.h"


unsigned limit = 255;

// Max Distance: 0.250200
int main() {;
    double range = 1.0;
    double maxDistance = range;
    for (double x = -range; x < range; x += 0.0001) {
        for (double y = -range; y < range; y += 0.0001) {
            double distance = sqrt(x * x + y * y);
            if(escapeTime(limit, x + y * I) < limit) {
                maxDistance = maxDistance > distance ? distance : maxDistance;
            }
        }
    }
    printf("Max Distance: %f\n", maxDistance);
}