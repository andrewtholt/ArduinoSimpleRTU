
#define TRUE 1
#define FALSE 0
#define UNKNOWN 2
#define RESERVED 3

#define INPUT 0
#define OUTPUT 1

#define UNO_DIO 14
#define UNO_ANALOG_IN 6


struct digitalPins {
    uint8_t state;
    uint8_t direction;
};



struct data {
    struct digitalPins ioPin[14];

    int16_t analogValue[UNO_ANALOG_IN];
};


