#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>
#include <sys/mman.h>
#include <string.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "message.h"

#include "data.h"

int main(int argc, char *argv[]) {

    sem_t *shmSem;
    sem_t *startSem;
    int shm;
    struct data *state;
    int qid;

    shmSem = sem_open( SHM_SEM,O_CREAT|O_RDWR,S_IRUSR|S_IWUSR,0);

    if( shmSem == SEM_FAILED ) {
        perror(SHM_SEM);
        exit(1);
    }

    startSem = sem_open( START_SEM,O_CREAT|O_RDWR,S_IRUSR|S_IWUSR,1);

    if( startSem == SEM_FAILED ) {
        perror(START_SEM);
        exit(2);
    }

    shm = shm_open(SHM_NAME, O_CREAT | O_RDWR, S_IRWXU | S_IRWXG);
    if( shm < 0 ) { 
        perror("shm-open");
        exit(4);
    } 

    ftruncate(shm, sizeof(struct data));
    state = (struct data *)mmap(NULL,sizeof(struct data) , PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0); 

    memset( state, sizeof(struct data), UNKNOWN);

    if ((qid = msgget(LISTENERQ, 0660)) == -1) {
        if ((qid = msgget(LISTENERQ, 0666 | IPC_CREAT)) == -1) {
            perror("msgget");
            exit(4);
        }
    }

    if ((qid = msgget(SENDERQ, 0660)) == -1) {
        if ((qid = msgget(SENDERQ, 0666 | IPC_CREAT)) == -1) {
            perror("msgget");
            exit(4);
        }
    }


    sem_post( shmSem );
    sem_close(shmSem);
    sem_close(startSem);

    exit(0);
}

