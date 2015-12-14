#include <stdint.h>
#include "common.h"

struct message {
    uint8_t address;  // defaults to 0
    uint8_t cmd[2];
    uint8_t item;
    uint8_t v_lo;
    uint8_t v_hi;
};
