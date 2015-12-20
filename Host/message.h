#include <stdint.h>
#include "common.h"

#define SENDERQ 42
#define LISTENERQ 43

struct message {
    uint8_t address;  // defaults to 0
    uint8_t cmd[2];
    uint8_t item;
    uint8_t v_lo;
    uint8_t v_hi;
};

struct msg {
    long mtype;
    struct message mtext;
};
