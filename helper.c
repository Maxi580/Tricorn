
#include <complex.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


int main() {
    float max = 2.0f;
    float min = -2.0f;
    float start_real = (max - min) * ((float) rand()/ (float) RAND_MAX);
    float start_imag = (max - min) * ((float) rand()/ (float) RAND_MAX);
    float res = 100.0f * ((float) rand()/ (float) RAND_MAX);

    printf("start_real: %f\n", start_real);
    printf("start_imag: %f\n", start_imag);
    printf("res: %f\n", res);
}