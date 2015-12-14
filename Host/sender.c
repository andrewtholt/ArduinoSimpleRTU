#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <mqueue.h>

#include "message.h"
#include "serialComms.h"

void usage() {
    printf("\n\tUsage\n");

    printf("\t-d              Debug Mode\n");
    printf("\t-h|-?           Help\n");
    printf("\t-p <port name>  Set serial Port\n");
    printf("\t-v              Verbose\n");

    exit(0);
}

int main(int argc,char *argv[]) {
    bool verbose=false;
    bool debug=false;
    bool runFlag=true;

    int opt;
    int ser;
    int len;

    struct message *cmd;

    char *serialPort=(char *)NULL;
    char buffer[8192];   // set this length for mq attributes
    char outBuffer[sizeof(struct message)];

    mqd_t clientCmds;

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

    clientCmds = mq_open("/SIMPLE_RTU", O_RDONLY|O_CREAT, S_IRUSR|S_IWUSR,NULL);
    if( clientCmds < 0) {
        perror("mq_open");
        exit(1);
    }

    while( runFlag ) {
        len = mq_receive( clientCmds, buffer, sizeof(buffer),NULL);
        if( len < 0) {
            perror("mq_receive");
            exit(3);
        }
        cmd = (struct message *)buffer;

        memset(outBuffer,0,sizeof(outBuffer));
        memcpy( outBuffer, cmd, sizeof(struct message));
        write( ser, outBuffer, sizeof(struct message));
    }
}

