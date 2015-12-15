#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "serialComms.h"
#include "message.h"
#include "data.h"

#define SHM_NAME "/RTUState"

void usage() {
    printf("\n\tUsage\n");

    printf("\t-d              Debug Mode\n");
    printf("\t-h|-?           Help\n");
    printf("\t-p <port name>  Set serial Port\n");
    printf("\t-v              Verbose\n");

    exit(0);
}

int myRead( int fd, uint8_t *ptr, int len) {

    int i=0;
    int c=0;

    do {
        i=read(fd,&ptr[c],1);
        if(i == 1) {
            printf("%d:%02x\n",i,ptr[c]);
            c++;
        }
    } while( c<len );

}

void displayState( struct data *ptr) {
    int i;
    uint16_t aValue;

    printf("Digital Pins\n\n");

    for(i=0;i<UNO_DIO;i++) {
        printf("\tPin %02d State:",i);

        switch( ptr->ioPin[i].state ) {
            case UNKNOWN:
                printf("Unknown ");
                break;
            case FALSE:
                printf("Off     ");
                break;
            case TRUE:
                printf("On      ");
                break;
            default:
                printf("ERROR   ");
                break;
        }
        printf("\n\t       Direction:");
        switch( ptr->ioPin[i].direction ) {
            case UNKNOWN:
                printf("Unknown ");
                break;
            case INPUT:
                printf("Input   ");
                break;
            case OUTPUT:
                printf("Output  ");
                break;
            default:
                printf("ERROR   ");
                break;
        }
        printf("\n");
    }

    for(i=0;i<UNO_ANALOG_IN;i++) {
        printf("\tADC %02d Value:%04d\n",i, ptr->analogValue[i]);
    }

}

int main(int argc,char *argv[]) {
    bool verbose=false;
    bool debug=false;
    bool runFlag=true;

    int opt;
    int ser;

    char *serialPort=(char *)NULL;
    char inBuffer[255];

    struct message *cmd;
    struct data *state;

    int shm;
    int i;

    uint16_t aValue;

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
    if( serialPort == (char *)NULL) {
        usage();
    }

    shm = shm_open(SHM_NAME, O_CREAT | O_RDWR, S_IRWXU | S_IRWXG);
    if( shm < 0 ) {
        perror("shm-open");
        exit(4);
    }

    ftruncate(shm, sizeof(struct data));

    state = (struct data *)mmap(NULL,sizeof(struct data) , PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0); 

    for(i=0;i<UNO_DIO;i++) {
        state->ioPin[i].state = UNKNOWN;
        state->ioPin[i].direction = UNKNOWN;
    }

    //    displayState( state );


    if(verbose) {
        printf("\nSerial Port:%s\n",serialPort);
    }

    ser = open (serialPort, O_RDONLY | O_NOCTTY | O_SYNC);

    if( 0 > ser) {
        printf("Failed to open serial port:%d\n",errno);
        perror("failed");
        exit(2);
    }

    setInterfaceAttribs (ser, B19200, 0);
    setBlocking (ser, 1);

    memset(inBuffer,0,sizeof(inBuffer));

    //    read( ser, inBuffer, 2);
    if( verbose ) {
        fprintf(stderr,"Waiting for STart\n");
    }

    myRead( ser, (uint8_t *)inBuffer, 2);

    if( inBuffer[0]=='S' && inBuffer[1]=='T' ) {
        if(verbose) {
            fprintf(stderr,"Start received\n");
        }
    }

    while(runFlag) {

        if( debug ) {
            displayState( state );
        }

        memset(inBuffer,0,sizeof(struct message));
        myRead( ser, (uint8_t *)inBuffer, sizeof(struct message));
        //        read(ser, inBuffer, sizeof(struct message));

        cmd = (struct message *) inBuffer;
        if(verbose) {
            fprintf(stderr,"Message RX\n");
            fprintf(stderr,"cmd\t%c%c\n", cmd->cmd[0], cmd->cmd[1]);
            fprintf(stderr,"Item\t%02d\n", cmd->item);
            fprintf(stderr,"v_lo\t%02d\n", cmd->v_lo);
            fprintf(stderr,"v_hi\t%02d\n", cmd->v_hi);
            fprintf(stderr,"-------------\n");
        }

        if( cmd->cmd[0] == 'W' ) {
            if(verbose) {
                fprintf(stderr,"Write ...\n");
            }
            if( cmd->cmd[1] == 'A' ) {
                if(verbose) {
                    fprintf(stderr,"... Analog\n");
                }
                aValue = cmd->v_lo & 0xff;
                aValue |= cmd->v_hi >> 8;

                state->analogValue[cmd->item] = aValue;
            } else if( cmd->cmd[1] == 'D' ) {
                if(verbose) {
                    fprintf(stderr,"... Digital\n");
                }
                state->ioPin[cmd->item].state = cmd->v_lo;
            } else if( cmd->cmd[1] == 'M' ) {
                if(verbose) {
                    fprintf(stderr,"... Mode\n");
                }
                state->ioPin[cmd->item].direction = cmd->v_lo;
            }

        }
    }
}

