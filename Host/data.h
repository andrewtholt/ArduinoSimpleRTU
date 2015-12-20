
#include "common.h"

#define ARDUINO_COUNT 16

#define SHM_SEM "/SHM_SEM"
#define START_SEM "/START_SEM"
#define SHM_NAME "/RTUState"


struct digitalPins {
    uint8_t state;
    uint8_t direction;
};



struct arduino {
    struct digitalPins ioPin[14];

    int16_t analogValue[UNO_ANALOG_IN];
};

struct data {
    struct arduino data[ARDUINO_COUNT];
};


