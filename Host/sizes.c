
#include <stdio.h>
#include <stdint.h>
#include "data.h"


int main() {

    printf("Shared Memory\n");

    printf("\tOverall     : %lu\n",sizeof(struct data));
    printf("\tDigital Pins: %lu\n",sizeof(struct digitalPins));
}


