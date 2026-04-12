#include <semaphore.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "utils.h"
#include <unistd.h>
#include <getopt.h>
#include <sys/time.h>
#include <math.h>
#include <string.h>


int main(int argc, char* argv[]){

    if(argc != 7){
        fprintf(stderr, "Usage: ./receptionist -d <orderTime> -s <sharedMemoryName> -l <logFileName.txt>\n");
        exit(EXIT_FAILURE);
    }
    int option;
    int maxOrderTime;
    char sharedMemoryName[64];
    char logFileName[64];

    while((option = getopt(argc, argv, "d:s:l:")) != -1){
        if(option == 'd'){
            maxOrderTime = atoi(optarg);
            if(maxOrderTime <= 0){
                fprintf(stderr, "Give valid number for orderTime please\n");
                exit(EXIT_FAILURE);
            }
        }
        else if(option == 's'){
            snprintf(sharedMemoryName, sizeof(sharedMemoryName), "/%s", optarg);
        }
        else if(option == 'l'){
            snprintf(logFileName, sizeof(logFileName), "%s", optarg);
        }
        else{
            fprintf(stderr, "Usage: ./receptionist -d <orderTime> -s <sharedMemoryName> -l <logFileName.txt>\n");
            exit(EXIT_FAILURE);
        }
    }

    int logFd = open(logFileName,  O_RDWR | O_APPEND , 0666);
    if(logFd == -1){
        perror("log file open failed");
        exit(EXIT_FAILURE);
    }
    int sharedFd;

    shareDataSegment* sharedData = attachShm(sharedMemoryName, &sharedFd);
    size_t sharedMemorySize = sizeof(shareDataSegment);
    struct timeval tv;
    gettimeofday(&tv, NULL);  // Get current time in seconds and microseconds
    // Combine seconds and microseconds for seed
    unsigned long seed = tv.tv_sec * 1000000 + tv.tv_usec;
    srand(seed); 


    while(1){

        // if there is no order to serve AND no one waiting inside the bar to be served AND tables are not occupied
        // AND bar is closing --------> exit (close the bar and clear the shared memory)

        sem_wait(&(sharedData->mutex));

        if(sharedData->closingFlag && 
        sharedData->fcfsWaitingBuffer.count == 0 &&
        sharedData->orderBuffer.count == 0 &&
        sharedData->tables[0].isOccupied == false && sharedData->tables[1].isOccupied == false &&
        sharedData->tables[2].isOccupied == false){
        
            sem_post(&(sharedData->mutex));
            break;  

        }

        sem_post(&(sharedData->mutex));

        // wait for an order to serve
        sem_wait(&(sharedData->receptionistSem));

        sem_wait(&(sharedData->mutex));
        
        // if there is order to serve
        if (sharedData->orderBuffer.count > 0) {

            int index = sharedData->orderBuffer.front;
            menuOrder currentOrder = sharedData->orderBuffer.lastOrders[index];

            updateStatistics(sharedData, currentOrder);


            int lower = (int)(ceil(0.5 * maxOrderTime));
            // random integer in the interval [maxOrderTime *0.5, maxOrderTime]
            int randomTime = lower + (rand() % (maxOrderTime - lower + 1));

            // free the mutex before sleeping for a random time
            sem_post(&(sharedData->mutex));
            
            // sleep for a random time -- preparing a speciffic order
            sleep(randomTime);


            sem_wait(&(sharedData->mutex));
            char buffer[256];
            sprintf(buffer, "\n[RECEPTIONIST] It took %d seconds to prepare the order of visitor with ID: %d\n", randomTime, currentOrder.visitor);
            write(logFd, buffer, strlen(buffer));

            // awake the first visitor in the queue of ordering in a specific chair FCFS,
            // from now on he can leave the bar after a random time(visitor source code)

            sem_post(&(sharedData->orderBuffer.chairSem[index]));
            // updating front (wrap around)
            sharedData->orderBuffer.front = (sharedData->orderBuffer.front + 1) % 12;
            sharedData->orderBuffer.count--;

        }

        sem_post(&(sharedData->mutex));
    }
    
    presentStatistics(sharedData);

    close(logFd);
    if (closingTheBar(sharedData, sharedMemoryName) != 0){
        perror("closingTheBar failed");
        exit(EXIT_FAILURE);
    }
    munmap(sharedData, sharedMemorySize);
    close(sharedFd);
    
    exit(EXIT_SUCCESS);
}