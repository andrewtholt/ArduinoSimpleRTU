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
#include <semaphore.h>

#include "serialComms.h"
#include "message.h"
#include "data.h"

// #define SHM_NAME "/RTUState"

// global debug flag
bool debug;

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
            if(debug) {
                printf("%d:%02x\n",i,ptr[c]);
            }
            c++;
        }
    } while( c<len );

}

void displayState( struct arduino *ptr) {
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
    bool runFlag=true;
    bool ok = false;

    int opt;
    int ser;

    char *serialPort=(char *)NULL;
    char inBuffer[255];
    char msgBuffer[ sizeof(struct message) * 2];

    struct message *cmd;
    struct data *state;

    int shm;
    int i;
    int rc=0;

    sem_t *shmSem;
    sem_t *startSem;
    key_t key=LISTENERQ;
    int qid;
    int len;

    struct msg out;

    uint16_t aValue;

    debug=false;
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


    shmSem = sem_open(SHM_SEM, O_RDWR);

    if(shmSem == SEM_FAILED) {
        perror("SHM_SEM");
        exit(5);
    }

    startSem = sem_open(START_SEM, O_RDWR);

    if(startSem == SEM_FAILED) {
        perror("START_SEM");
        exit(5);
    }

    shm = shm_open(SHM_NAME, O_RDWR, S_IRWXU | S_IRWXG);
    if( shm < 0 ) {
        perror("shm-open");
        exit(4);
    }

    ftruncate(shm, sizeof(struct data));

    state = (struct data *)mmap(NULL,sizeof(struct data) , PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0); 

    /*
       for(i=0;i<UNO_DIO;i++) {
       state->data[0].ioPin[i].state = UNKNOWN;
       state->data[0].ioPin[i].direction = UNKNOWN;
       }
       */

    if(debug) {
    }


    if(verbose) {
        // displayState( &(state->data[0]) );
        printf("\nSerial Port:%s\n",serialPort);
    }

    // sem_wait(startSem);

    ser = open (serialPort, O_RDONLY | O_NOCTTY | O_SYNC);

    if( 0 > ser) {
        printf("Failed to open serial port:%d\n",errno);
        perror("failed");
        rc++;
        //        exit(2);
    } else {

        // setInterfaceAttribs (ser, B19200, 0);
        setInterfaceAttribs (ser, B115200, 0);
        setBlocking (ser, 1);

        memset(inBuffer,0,sizeof(inBuffer));
        memset(msgBuffer,0,sizeof(msgBuffer));


        //    read( ser, inBuffer, 2);
        if( verbose ) {
            fprintf(stderr,"Waiting for Start\n");
        }

        myRead( ser, (uint8_t *)inBuffer, 2);

        if( inBuffer[0]=='S' && inBuffer[1]=='T' ) {
            if(verbose) {
                fprintf(stderr,"Start received\n");
            }
        }
    }
    sem_post(startSem);

    if( rc != 0) {
        runFlag = false;
    } else {
        if ((qid = msgget(key, 0660)) == -1) {
            perror("listener:msgget");
            exit(4);
        }
    }

    while(runFlag) {

        if( debug ) {
            // displayState( &(state->data[0]) );
        }

        printf("sizeof message %d\n", (int)sizeof(struct message));
        memset(inBuffer,0,sizeof(struct message));
        myRead( ser, (uint8_t *)inBuffer, sizeof(struct message));
        //        read(ser, inBuffer, sizeof(struct message));

        cmd = (struct message *) inBuffer;

        sprintf(msgBuffer,"%c%c%02d%02d%02d\n", 
                cmd->cmd[0], cmd->cmd[1], cmd->item, cmd->v_lo, cmd->v_hi);

        if(verbose) {
            fprintf(stderr,"Message RX\n");
            fprintf(stderr,"Sending\n\t>%s<\n", msgBuffer);

            fprintf(stderr,"cmd\t%c%c\n", cmd->cmd[0], cmd->cmd[1]);
            fprintf(stderr,"Item\t%02d\n", cmd->item);
            fprintf(stderr,"v_lo\t%02d\n", cmd->v_lo);
            fprintf(stderr,"v_hi\t%02d\n", cmd->v_hi);
            fprintf(stderr,"-------------\n");
        }

        ok = false;

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

                state->data[0].analogValue[cmd->item] = aValue;
                ok = true;
            } else if( cmd->cmd[1] == 'D' ) {
                if(verbose) {
                    fprintf(stderr,"... Digital\n");
                }
                state->data[0].ioPin[cmd->item].state = cmd->v_lo;
                ok = true;
            } else if( cmd->cmd[1] == 'M' ) {
                if(verbose) {
                    fprintf(stderr,"... Mode\n");
                }
                state->data[0].ioPin[cmd->item].direction = cmd->v_lo;
                ok = true;
            }

            if( ok ) {
                out.mtype = 1;
                
                memcpy(&(out.mtext), cmd, sizeof(struct message));
                len = msgsnd(qid, &out,sizeof(struct message), 0);
                if ( len < 0) {
                    perror("msgsnd");
                    exit(6);
                }
            }
        }
    }
}

