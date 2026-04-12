#include <semaphore.h>
#include "utils.h"
#include <sys/mman.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char* argv[]){

    if(argc != 3){
        fprintf(stderr, "Usage: ./closer -s sharedMemoryName\n");
        exit(EXIT_FAILURE);
    }
    int option;
    char sharedMemoryName[64];

    while((option = getopt(argc, argv, "s:")) != -1){
        if(option == 's'){
            snprintf(sharedMemoryName, sizeof(sharedMemoryName), "/%s", optarg);
        }
        else{
            fprintf(stderr, "Usage: ./closer -s sharedMemoryName\n");
            exit(EXIT_FAILURE);
        }
    }

    int sharedFd;
    shareDataSegment* sharedData = attachShm(sharedMemoryName, &sharedFd);

    size_t sharedMemorySize = sizeof(shareDataSegment);

    if(sharedData->closingFlag == 0){
        sem_wait(&(sharedData->mutex));
        sharedData->closingFlag = true;

        if(sharedData->fcfsWaitingBuffer.count == 0 &&
        sharedData->orderBuffer.count == 0 &&
        sharedData->tables[0].isOccupied == false && sharedData->tables[1].isOccupied == false &&
        sharedData->tables[2].isOccupied == false){

            //if there is no one in the bar, just inform the receptionist that he should close the bar
            sem_post(&(sharedData->receptionistSem));
        }

        sem_post(&(sharedData->mutex));

    }

    munmap(sharedData, sharedMemorySize);
    close(sharedFd);
    exit(EXIT_SUCCESS);
}