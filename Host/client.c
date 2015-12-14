#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stdint.h>

#include "serialComms.h"

// extern int errno;
extern int enQueue(char *,char *,int );

void usage() {
    printf("\n\tUsage\n");

    printf("\t-d              Debug Mode\n");
    printf("\t-h|-?           Help\n");
    printf("\t-p <port name>  Set serial Port\n");
    printf("\t-v              Verbose\n");

    exit(0);
}


int main( int argc, char *argv[]) {
    int opt;
    bool debug =  false;
    bool verbose = false;
    char *serialPort=(char *)NULL;
    int ser;

    char outBuffer[255];
    char inBuffer[255];

    bool exitFlag = false;
    char *ptr;
    int len;
    char test[3];
    uint8_t idx=0;

    while ((opt = getopt(argc, argv, "dh?p:v")) != -1) {
        switch(opt) {
            case 'd':
                debug=true;
                break;
            case 'v':
                verbose=true;
                printf("\nVerbose mode on.\n");
                break;
            case 'h':
            case '?':
                usage();
                break;
            case 'p':
                serialPort = (char *)malloc(strlen(optarg)+1);

                if( (char *)NULL == serialPort) {
                    perror("Failed to allocate memory for serial port");
                    exit(1);
                }
                strcpy(serialPort, optarg);
                break;
            default:
                break;
        }

    }

    if(verbose) {
        printf("\nSerial Port:%s\n",serialPort);
    }
    ser = open (serialPort, O_RDWR | O_NOCTTY | O_SYNC);

    if( 0 > ser) {
        printf("Failed to open serial port:%d\n",errno);
        perror("failed");
        exit(2);
    }

    setInterfaceAttribs (ser, B19200, 0);
    setBlocking (ser, 1);

    idx=0;
    memset(inBuffer,0,sizeof(inBuffer));
    read( ser, inBuffer, 2);

    if(!strncmp(inBuffer,"ST",2)) {
        printf("Start\n");
    }

    outBuffer[0] = 'S';
    outBuffer[1] = 'M';
    outBuffer[2] = 13;
    outBuffer[3] = 1;
    outBuffer[4] = 0;

    write( ser, outBuffer, 5);

    while( exitFlag == false ) { 
    }


    return(0);
}
