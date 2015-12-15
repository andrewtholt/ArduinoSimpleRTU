#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
////  #include <mqueue.h>

#include <sys/ipc.h>
#include <sys/msg.h>

#include "message.h"
#include "serialComms.h"

void usage() {
    printf("\n\tUsage\n");

    printf("\t-d              Debug Mode\n");
    printf("\t-h|-?           Help\n");
    printf("\t-p <port name>  Set serial Port\n");
    printf("\t-q <qid>        Message queue key\n");
    printf("\t-v              Verbose\n");

    exit(0);
}

struct msg {
    long mtype;
    struct message mtext;
};

int main(int argc,char *argv[]) {
    bool verbose=false;
    bool debug=false;
    bool runFlag=true;

    int opt;
    int ser;
    int len;

    key_t key=42;
    int qid;

    struct message *cmd;

    char *serialPort=(char *)NULL;
    struct msg buffer;
    char outBuffer[sizeof(struct message)];

    //    mqd_t clientCmds;

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
            case 'q':
                key = atoi( optarg );
                break;
            default:
                break;
        }

    }

    if( serialPort == (char *)NULL) {
        usage();
    }
    if(verbose) {
        printf("\nSerial Port:%s\n",serialPort);
    }
    ser = open (serialPort, O_WRONLY | O_NOCTTY | O_SYNC);

    if( 0 > ser) {
        printf("Failed to open serial port:%d\n",errno);
        perror("failed");
        exit(2);
    }

    setInterfaceAttribs (ser, B19200, 0);
    setBlocking (ser, 1);

    if ((qid = msgget(key, 0660)) == -1) {
        if ((qid = msgget(key, 0666 | IPC_CREAT)) == -1) {
            perror("msgget");
            exit(4);
        }
    }

    while( runFlag ) {
        //        len = mq_receive( clientCmds, buffer, sizeof(buffer),NULL);
        len = msgrcv(qid, &buffer,sizeof(struct message), 1, 0);
        if( len < 0) {
            perror("mq_receive");
            exit(3);
        }
        cmd = &(buffer.mtext);
        if(verbose) {
            fprintf(stderr,"Message RX\n");
            fprintf(stderr,"Addr\t%02d\n", cmd->address);
            fprintf(stderr,"cmd\t%c%c\n", cmd->cmd[0], cmd->cmd[1]);
            fprintf(stderr,"Item\t%02d\n", cmd->item);
            fprintf(stderr,"v_lo\t%02d\n", cmd->v_lo);
            fprintf(stderr,"v_hi\t%02d\n", cmd->v_hi);
            fprintf(stderr,"-------------\n");
        }   

        memset(outBuffer,0,sizeof(outBuffer));
        memcpy( outBuffer, cmd, sizeof(struct message));
        write( ser, outBuffer, sizeof(struct message));
    }
}

