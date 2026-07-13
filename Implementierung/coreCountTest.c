#include <stdio.h>
#include "corecalculator.h"

int main() {
    int cores = getCoreAmount();
    printf("Anzahl der cores: %u\n", cores);

    return 0;
}
