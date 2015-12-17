#include "Host/message.h"
#include "Host/common.h"

#include <SoftwareSerial.h>
#include <EEPROM.h>

#define ADDRESS 0       // EEPROM location to hold ...
#define MY_ADDRESS 0    // ... my address

#define CONFIG 1        // EEPROM location to hold config
#define DEFAULT_CONFIG 0x00
#define ENABLE_SCAN 0x01
#define ENABLE_ANALOG_SCAN 0x02
#define ENABLE_DEBUG_OUT 0x04

// 
// Config Byte
// 7 - 0
// 6
// 5
// 4
// 3
// 2 - 0 Disable o/p to soft serial port., 1 Enable
// 1 - 0 Disable Analog Scan, 1 Enable.
// 0 - 0 Disable scan, 1 enable scan
//

#define SCAN_CONFIG_LO 4
#define SCAN_CONFIG_HI 5

#define ANALOG_SCAN_CONFIG_LO 6
#define ANALOG_SCAN_CONFIG_HI 7

#define USER_EEPROM_OFFSET 0x10	// Start of user EEPROM 

#define DEFAULT_SCAN_DELAY_LO 100  // Report inputs every ... in multiples of 
                                   // 10 ms
#define DEFAULT_SCAN_DELAY_HI 0  

uint8_t pin[UNO_DIO];
uint16_t analogIn[UNO_ANALOG_IN];

SoftwareSerial mySerial(8,9);  // RX, TX

void flush() {
    while( Serial.available() > 0) {
        Serial.read();
    }
}

bool scanEnabled() {
    return( (bool) (EEPROM.read(CONFIG) & ENABLE_SCAN));
}

unsigned int getScanDelay() {
    uint8_t lo;
    uint8_t hi;
    unsigned int delay;

    lo = EEPROM.read(DEFAULT_SCAN_DELAY_LO);
    hi = EEPROM.read(DEFAULT_SCAN_DELAY_HI);

    delay = ( (hi << 8) | (lo & 0xff))*10;
    return( (hi << 8) | (lo & 0xff));
}

void setScanDelay(uint8_t hi, uint8_t lo) {
    EEPROM.write(DEFAULT_SCAN_DELAY_HI,hi);
    EEPROM.write(DEFAULT_SCAN_DELAY_LO,lo);
}

void setup() {
    int i=0;

    mySerial.begin(9600);
    mySerial.println("Debug Ready");

    if( EEPROM.read( ADDRESS ) == 0xff ) {
        mySerial.println("Address Set");
        EEPROM.write(ADDRESS, MY_ADDRESS);
    }

    if( EEPROM.read(CONFIG) == 0xff) {
        EEPROM.write(CONFIG, DEFAULT_CONFIG);
    }

    if( EEPROM.read(SCAN_CONFIG_LO) == 0xff) {
        EEPROM.write(SCAN_CONFIG_LO, DEFAULT_SCAN_DELAY_LO);
        EEPROM.write(SCAN_CONFIG_HI, DEFAULT_SCAN_DELAY_HI);
    }

    for( i = 0; i<UNO_DIO;i++) {
        pin[i]=UNKNOWN;
    }

    for( i = 0; i<UNO_ANALOG_IN;i++) {
        analogIn[i]=0xffff;
    }

    Serial.begin(19200);

    flush();
    Serial.print("ST");
}

void getBytes(uint8_t *ptr, uint8_t count) {
    int i=0;
    int c;

    while( Serial.available() < count);

    for(i=0;i<count;i++) {
        c=Serial.read();
        mySerial.println(c, HEX) ;
        ptr[i]=c;
    }
}

void sendBytes(uint8_t *ptr, uint8_t count) {
    int i;

    for(i=0;i<count;i++) {
        Serial.write(ptr[i]);
    }
}

// Set Scan Delay (units of 10 ms)
void handleSD( struct message *cmd) {
    mySerial.println("->SD");

    getBytes(&cmd->item,1);
    getBytes(&cmd->v_lo,1);  
    getBytes(&cmd->v_hi,1);  

    if( cmd->item == 0) {
        setScanDelay(cmd->v_hi, cmd->v_lo);
    }   
}
// Set config byte
void handleSC( struct message *cmd) {
    mySerial.println("->SC");

    getBytes(&cmd->item,1);
    getBytes(&cmd->v_lo,1);  
    getBytes(&cmd->v_hi,1);  

    if( cmd->item == 0) {
        EEPROM.write( CONFIG, cmd->v_lo);
    }   
}

void handleSM( struct message *cmd) {
    mySerial.println("->SM");

    // data[0] is the bit number, and data[1] is the direction
    // 1 is o/p 0 is i/p
    //
    getBytes(&cmd->item,1);
    getBytes(&cmd->v_lo,1);  
    getBytes(&cmd->v_hi,1);  

    if ( cmd->v_lo == 0 ) {
        pinMode(cmd->item, INPUT);
    } else if(cmd->v_lo == 1) {
        pinMode(cmd->item, OUTPUT);
    }
    cmd->address = MY_ADDRESS;
    cmd->cmd[0] = 'W';
    sendBytes( (uint8_t*)cmd,sizeof(struct message));
}

// Write Digital
void handleWD( struct message *cmd ) {
    mySerial.println("->WD");
    getBytes(&cmd->item,1);
    getBytes(&cmd->v_lo,1);  
    getBytes(&cmd->v_hi,1);  

    digitalWrite( cmd->item, cmd->v_lo );
    sendBytes( (uint8_t*)cmd,sizeof(struct message));
}

// Read Analog
void handleRA( struct message *cmd) {
    uint16_t analogValue;

    mySerial.println("->RA");

    getBytes(&cmd->item,1);
    getBytes(&cmd->v_lo,1);  
    getBytes(&cmd->v_hi,1);  

    analogValue = analogRead(cmd->item);
    analogIn[cmd->item] = analogValue;

    cmd->cmd[0] = 'W';
    cmd->v_lo = analogValue & 0xff;
    cmd->v_hi = (analogValue >> 8) & 0xff;
    sendBytes( (uint8_t*)cmd,sizeof(struct message));
}

// Read Digital
void handleRD( struct message *cmd ) {
    mySerial.println("->RD");
    cmd->cmd[0] = 'W';

    getBytes(&cmd->item,1);
    getBytes(&cmd->v_lo,1);  
    getBytes(&cmd->v_hi,1);  

    cmd->v_lo = digitalRead(cmd->item);
    cmd->v_hi = 0x00;
    sendBytes( (uint8_t*)cmd,sizeof(struct message));
    pin[cmd->item] = 0;
}

unsigned long previousMillis =0;

void loop() {
    //    uint8_t cmd[2];
    uint8_t item;
    uint8_t data[2];
    struct message cmd;
    unsigned long currentMillis = millis();

    //    flush();
    //    mySerial.println("Top");
    if( Serial.available() > 0) {

        //    getBytes(&cmd[0],2);
        getBytes(&cmd.address,1);
        getBytes(&cmd.cmd[0],2);

        if(cmd.cmd[0] == 'S' && cmd.cmd[1] == 'M') {
            // 
            // Set Mode of pin
            //
            handleSM(&cmd);

        } else if(cmd.cmd[0] == 'S' && cmd.cmd[1] == 'D' ) {
            handleSD( &cmd );
        } else if(cmd.cmd[0] == 'S' && cmd.cmd[1] == 'C' ) {
            handleSC( &cmd );

        } else if(cmd.cmd[0] == 'W' && cmd.cmd[1] == 'D' ) {
            // 
            // Write to digital OP
            //
            handleWD( &cmd );

        } else if(cmd.cmd[0] == 'R' && cmd.cmd[1] == 'A' ) {
            // 
            // Read ananlog
            //
            handleRA( &cmd );
        } else if(cmd.cmd[0] == 'R' && cmd.cmd[1] == 'D' ) {
            // 
            // Read digital
            //
            handleRD( &cmd );

        }
    } else if(scanEnabled()) {
        if( currentMillis - previousMillis >= getScanDelay() ) {
            previousMillis = currentMillis ;

            for(int i=0;i<UNO_DIO;i++) {
                if( pin[i] != UNKNOWN) {
                    mySerial.println("Digital");
                    uint8_t tmp = digitalRead(i);

                    mySerial.println( tmp,pin[i] );
                    if( tmp != pin[i] ) {
                        mySerial.println( pin[i] );
                        pin[i] = tmp;

                        cmd.address=0;
                        cmd.cmd[0]='W';
                        cmd.cmd[1]='D';
                        cmd.item = i;
                        cmd.v_lo = tmp;
                        cmd.v_hi = 0;
                        sendBytes( (uint8_t*)&cmd,sizeof(struct message));
                    }
                }
            }
            mySerial.println("=====================");

            for(int i=0;i<UNO_ANALOG_IN;i++) {
                if(analogIn[i] != 0xffff) {
                    mySerial.println("Analog");
                    uint16_t analogValue;
                    analogValue = analogRead(i);

                    cmd.address=0;
                    cmd.cmd[0]='W';
                    cmd.cmd[1]='A';
                    cmd.item = i;

                    cmd.v_lo = analogValue & 0xff;
                    cmd.v_hi = (analogValue >> 8) & 0xff;
                    sendBytes( (uint8_t*)&cmd,sizeof(struct message));
                }
            }
        }
    }
}






