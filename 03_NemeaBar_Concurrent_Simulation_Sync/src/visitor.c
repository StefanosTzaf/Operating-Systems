#include <stdio.h>
#include <semaphore.h>
#include <stdlib.h>
#include "utils.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/time.h>
#include <sys/times.h>
#include <string.h>
#include <math.h>


int main(int argc, char* argv[]){

    if(argc != 7){
        fprintf(stderr, "Usage: ./visitor -d <restTime> -s <sharedMemoryName> -l <logFileName.txt>\n");
        exit(EXIT_FAILURE);
    }
    int option;
    int maxRestTime;
    char sharedMemoryName[64];
    char logFileName[64];

    while((option = getopt(argc, argv, "d:s:l:")) != -1){
        if(option == 'd'){
            maxRestTime = atoi(optarg);
        }
        else if(option == 's'){
            snprintf(sharedMemoryName, sizeof(sharedMemoryName), "/%s", optarg);
        }
        else if(option == 'l'){
            snprintf(logFileName, sizeof(logFileName), "%s", optarg);
        }
        else{
            fprintf(stderr, "Usage: ./visitor -d <restTime> -s <sharedMemoryName> -l <logFileName.txt>\n");
            exit(EXIT_FAILURE);
        }
    }

    int logFd = open(logFileName, O_RDWR | O_APPEND , 0666);
    if(logFd == -1){
        perror("log file open failed");
        exit(EXIT_FAILURE);
    }


    double totalTimeInBar_1, totalTimeInBar_2, totalTimeWaiting_1, totalTimeWaiting_2;
    double ticspersec;
    ticspersec = (double) sysconf (_SC_CLK_TCK);

    //to be able to close it at the end of the process
    int sharedFd;
    shareDataSegment* sharedData = attachShm(sharedMemoryName, &sharedFd);

    size_t sharedMemorySize = sizeof(shareDataSegment);

    // fully random seed based to time
    struct timeval tv;
    gettimeofday(&tv, NULL);
    unsigned long seed = tv.tv_sec * 1000000 + tv.tv_usec ;
    srand(seed);

    //logging data
    char buffer[256];
    sprintf(buffer,"\n[INFO] Visitor with ID: %d has just arrived\n", getpid());
    write(logFd, buffer, strlen(buffer));

    if(sharedData->closingFlag){
        fprintf(stdout, "Sorry bar is closing\n");
        sprintf(buffer,"\n[LEAVING] Visitor with ID: %d has just left because bar is closing\n", getpid());
        write(logFd, buffer, strlen(buffer));
        munmap(sharedData, sharedMemorySize);
        close(logFd);
        exit(EXIT_SUCCESS);
    } 


    // start counting the total time that visitor waited in the bar till he takes a seat
    totalTimeWaiting_1 = (double) times (NULL);

    // if there is no place for visitor to wait inside the bar they should wait outside.
    // So if in this semaphore P() has been applied more than MAX_VISITORS times then the fcfsWaitingBuffer is full 
    sem_wait(&(sharedData->exceedingVisitorsSem));
    

    // if the execution has reached here, there is space in waiting buffer OR the waiting buffer is absolutely free
    // and the bar is NOT closing

    sem_wait(&(sharedData->mutex));
    //start counting the total time that visitor stayed in the bar

    int chairIndex;
    int tableIndex = isAnyTableEmpty(sharedData);

    // if no one is waiting in the buffer he can check in the bar, if there is space in a table and is not occupied sit there
    if(sharedData->fcfsWaitingBuffer.count == 0 && tableIndex != -1){

        // visitor did not wait in the buffer so exceeding buffer semaphore should be incremented
        sem_post(&(sharedData->exceedingVisitorsSem));

        totalTimeWaiting_2 = (double) times (NULL);

        sitInTheFirstEmptyChair(sharedData, getpid(), tableIndex);
        totalTimeInBar_1 = (double) times (NULL);

        menuOrder order = randomizeOrder(getpid(), logFd);

        // put the order in the order buffer
        sharedData->orderBuffer.lastOrders[sharedData->orderBuffer.back] = order;

        // chair to sleep 
        chairIndex = sharedData->orderBuffer.back;
        sharedData->orderBuffer.back = (sharedData->orderBuffer.back + 1) % 12;
        sharedData->orderBuffer.count++;

    }
    
    // if there is no space in the tables or is someone waiting in the buffer just wait at the buffer's end (FCFS)
    else{
        int position = sharedData->fcfsWaitingBuffer.back;
        sharedData->fcfsWaitingBuffer.buffer[position] = getpid();
        sharedData->fcfsWaitingBuffer.back = (sharedData->fcfsWaitingBuffer.back + 1) % MAX_VISITORS;
        sharedData->fcfsWaitingBuffer.count++;

        sem_post(&(sharedData->mutex));
        //suspend in this semaphore after unlocking mutex
        sem_wait(&(sharedData->fcfsWaitingBuffer.positionSem[position]));


        // here the visitor has been awaken by a last leaving visitor so this visitor has incremeted the excededing buffer semaphore not here
        sem_wait(&(sharedData->mutex));

        tableIndex = isAnyTableEmpty(sharedData);

        totalTimeWaiting_2 = (double) times (NULL);
        sitInTheFirstEmptyChair(sharedData, getpid(), tableIndex);
        //start counting the total time that visitor stayed in the bar
        totalTimeInBar_1 = (double) times (NULL);

        //when he awakes, he can order
        menuOrder order = randomizeOrder(getpid(), logFd);
        sharedData->orderBuffer.lastOrders[sharedData->orderBuffer.back] = order;
        chairIndex = sharedData->orderBuffer.back;
        sharedData->orderBuffer.back = (sharedData->orderBuffer.back + 1) % 12;
        sharedData->orderBuffer.count++;

    }

    // inform receptionist that there is an order to serve
    sem_post(&(sharedData->receptionistSem));

    sem_post(&(sharedData->mutex));

    // it should be suspended in the semaphore of his chair (after mutex unlock)
    sem_wait(&(sharedData->orderBuffer.chairSem[chairIndex]));



    sem_wait(&(sharedData->mutex));
    // now the visitor should have been served (because only receptionist can awake him from the semaphore with orders!!)
    //so it has to wait a random time -- eat and discuss --and then leave the bar

    int lower = (int)(ceil(0.7 * maxRestTime));
 
    sprintf(buffer, "\n[SERVED] Visitor with ID: %d has been served and is now eating. \n", getpid());
    write(logFd, buffer, strlen(buffer));

    // random integer in the interval [lower *0.7, maxRestTime]
    int randomTime = lower + (rand() % (maxRestTime - lower + 1));

    sem_post(&(sharedData->mutex));

    sleep(randomTime);
    
    
    sem_wait(&(sharedData->mutex));

    // visitor just left the bar 
    totalTimeInBar_2 = (double) times (NULL);

    sharedData->sharedStatistics.totalStayTime += (totalTimeInBar_2 - totalTimeInBar_1) / ticspersec;
    sharedData->sharedStatistics.totalWaitingTime += (totalTimeWaiting_2 - totalTimeWaiting_1) / ticspersec;

    // visitor has finished eating and is leaving the bar
    sprintf(buffer, "\n[LEAVE] Visitor with ID: %d was eating for %d seconds and has just left the bar\n", getpid(), randomTime);
    write(logFd, buffer, strlen(buffer));
    
    // defensive case
    if(tableIndex != -1){
        sharedData->tables[tableIndex].chairsOccupied--;
        //inform others that the chair is empty
        int chair = findChairInTable(sharedData, getpid(), tableIndex);
        sharedData->tables[tableIndex].chairs[chair] = -1;


        if(sharedData->tables[tableIndex].chairsOccupied == 0 && sharedData->tables[tableIndex].isOccupied){
            lastVisitorInformingOthers(sharedData, tableIndex);
        }
    }

    sem_post(&(sharedData->mutex));


    close(logFd);
    munmap(sharedData, sharedMemorySize);
    close(sharedFd);
    exit(EXIT_SUCCESS);

}