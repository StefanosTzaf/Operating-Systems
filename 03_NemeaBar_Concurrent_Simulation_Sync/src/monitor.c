#include "utils.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>



int main(int argc, char* argv[]){
    if(argc != 3){
        fprintf(stderr, "Usage: ./monitor -s sharedMemoryName\n");
        exit(EXIT_FAILURE);
    }
    int option;
    char sharedMemoryName[64];
    if ((option = getopt(argc, argv, "s:")) != -1) {
        if(option == 's'){
            snprintf(sharedMemoryName, sizeof(sharedMemoryName), "/%s", optarg);
        }
        else{
            fprintf(stderr, "Usage: ./monitor -s sharedMemoryName\n");
            exit(EXIT_FAILURE);
        }
    }

    int sharedFd;
    shareDataSegment* sharedData = attachShm(sharedMemoryName, &sharedFd);
    size_t sharedMemorySize = sizeof(shareDataSegment);
    
    sem_wait(&(sharedData->mutex));

    printf("\n");
    printf("Visitors waiting in the queue: %d\n", sharedData->fcfsWaitingBuffer.count);
    printf("\n\n");

    for(int i = 0; i < 3; i++) {
        printf("Table %d -> ", i);
        if(sharedData->tables[i].isOccupied) {
            printf("Occupied\n");
        }
        else{
            printf("Unoccupied\n");
        }
        printf("Chairs occupied on this table: %d\n", sharedData->tables[i].chairsOccupied);
        for(int j = 0; j < 4; j++) {
            if(sharedData->tables[i].chairs[j] != -1) {
                printf("   Visitor : %d is sitting on chair %d\n", sharedData->tables[i].chairs[j], j);
            }
        }
        printf("\n\n");
    }

    printf("\n            Total products consumed by %d visitors:\n\n",sharedData->sharedStatistics.visitorsServed); 

    printf("Water:  %d\n", sharedData->sharedStatistics.consumedWater);
    printf("Wine:   %d\n", sharedData->sharedStatistics.consumedWine);
    printf("Cheese: %d\n", sharedData->sharedStatistics.consumedCheese);
    printf("Salads: %d\n", sharedData->sharedStatistics.consumedSalads);
    if(sharedData->sharedStatistics.visitorsServed == 0){
        printf("Average waitng time until now: 0\n");
        printf("Average time inside the bar until now: 0\n\n");
    }
    else{
        printf("Average waitng time until now: %.5f\n", sharedData->sharedStatistics.totalWaitingTime / sharedData->sharedStatistics.visitorsServed);
        printf("Average time inside the bar until now: %.5f\n\n", sharedData->sharedStatistics.totalStayTime / sharedData->sharedStatistics.visitorsServed);
    }
    sem_post(&(sharedData->mutex));
    
    munmap(sharedData, sharedMemorySize);
    close(sharedFd);
    exit(EXIT_SUCCESS);
}