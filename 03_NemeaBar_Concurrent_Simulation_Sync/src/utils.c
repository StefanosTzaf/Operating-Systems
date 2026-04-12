#include "utils.h"
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void initializeSharedValues(shareDataSegment *sharedData) {

    sharedData->sharedStatistics.totalWaitingTime = 0.0;
    sharedData->sharedStatistics.totalStayTime = 0.0;
    sharedData->sharedStatistics.consumedWine = 0;
    sharedData->sharedStatistics.consumedWater = 0;
    sharedData->sharedStatistics.consumedCheese = 0;
    sharedData->sharedStatistics.consumedSalads = 0;
    sharedData->sharedStatistics.visitorsServed = 0;
    

    // Initialize the FCFS waiting circular buffer
    sharedData->fcfsWaitingBuffer.front = 0;
    sharedData->fcfsWaitingBuffer.back = 0;
    sharedData->fcfsWaitingBuffer.count = 0;
    for (int i = 0; i < MAX_VISITORS; i++) {

        // Initialize the semaphores for each position to 0, the first visitor will be suspended there
        sem_init(& (sharedData->fcfsWaitingBuffer.positionSem[i]), 1, 0);
        sharedData->fcfsWaitingBuffer.buffer[i] = -1;

    }

    sem_init(&sharedData->exceedingVisitorsSem, 1, MAX_VISITORS);

    // Initialize the order circular buffer
    sharedData->orderBuffer.front = 0;
    sharedData->orderBuffer.back = 0;
    sharedData->orderBuffer.count = 0;
    for (int i = 0; i < 12; i++) {
        sem_init(&sharedData->orderBuffer.chairSem[i], 1, 0);
        sharedData->orderBuffer.lastOrders[i].visitor = -1;
        sharedData->orderBuffer.lastOrders[i].water = false;
        sharedData->orderBuffer.lastOrders[i].wine = false;
        sharedData->orderBuffer.lastOrders[i].cheese = false;
        sharedData->orderBuffer.lastOrders[i].salad = false;
    }

    // Initialize the tables
    for (int i = 0; i < 3; i++) {
        sharedData->tables[i].isOccupied = false; // All tables are unoccupied initially
        sharedData->tables[i].chairsOccupied = 0;
        for(int j = 0; j < 4; j++) {
            sharedData->tables[i].chairs[j] = -1; // No visitor is sitting on any chair initially
        }
    }


    // Initialize the mutex and receptionist semaphore
    sem_init(&sharedData->mutex, 1, 1);
    sem_init(&sharedData->receptionistSem, 1, 0); // Receptionist will be asleep until there is work

    // Set the closing flag to false initially
    sharedData->closingFlag = false;
}


// Function to attach shared memory
shareDataSegment* attachShm(char* sharedMemoryName, int* sharedFd){
    int shmFd;
    size_t sharedMemorySize = sizeof(shareDataSegment);
    shareDataSegment* sharedData;

    // Open shared memory
    shmFd = shm_open(sharedMemoryName, O_RDWR, 0666);
    if (shmFd == -1) {
        perror("shared memory open failed");
        exit(EXIT_FAILURE);
    }

    // Map shared memory in current address space
    sharedData = mmap(0, sharedMemorySize, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
    if (sharedData == MAP_FAILED) {
        perror("mmap failed");
        exit(EXIT_FAILURE);
    }
    *sharedFd = shmFd;
    return sharedData;
}


// return the first empty table index, else return -1
int isAnyTableEmpty(shareDataSegment* sharedData){
    for(int i = 0; i < 3; i++){
        if(sharedData->tables[i].isOccupied == false){
            return i;
        }
    }
    return -1;
}


menuOrder randomizeOrder(pid_t visitorID, int logFd) {
    menuOrder order;
    order.visitor = visitorID;

    // Randomize drinks - at least one drink is selected
    // 0 = water only, 1 = wine only, 2 = both
    int drinkChoice = rand() % 3;
    if(drinkChoice == 0) {
        order.wine = false;
        order.water = true;
    }
    else if(drinkChoice == 1) {
        order.wine = true;
        order.water = false;
    }
    else {
        order.wine = true;
        order.water = true;
    }

    // Randomize food - 0 to 2 food items (it is possible for visitor not to order anything for food)
    // 0 = nothing, 1 = cheese, 2 = salad, 3 = both
    int foodChoice = rand() % 4;

    if(foodChoice == 0){
        order.cheese = false;
        order.salad = false;
    }
    else if(foodChoice == 1){
        order.cheese = true;
        order.salad = false;
    }
    else if(foodChoice == 2){
        order.salad = true;
        order.cheese = false;
    }
    else if(foodChoice == 3){
        order.cheese = true;
        order.salad = true;
    }

    //log file update
    char logMessage[256];
    snprintf(logMessage, sizeof(logMessage), 
             "\n[ORDER] Visitor with ID: %d ordered %s%s%s%s",
             visitorID,
             order.cheese ? "cheese, " : "",
             order.salad ? "salad, " : "",
             order.wine ? "wine, " : "",
             order.water ? "water, " : "");

    // Remove , and add new line

    if(logMessage[strlen(logMessage) - 2] == ','){
        logMessage[strlen(logMessage) - 2] = '\n';
        logMessage[strlen(logMessage) - 1] = '\0';
    }

    write(logFd, logMessage, strlen(logMessage));

    return order;
}

void updateStatistics(shareDataSegment* sharedData, menuOrder currentOrder){
    sharedData->sharedStatistics.visitorsServed++;

    if (currentOrder.water) {
        sharedData->sharedStatistics.consumedWater++;
    }
    if(currentOrder.wine){
        sharedData->sharedStatistics.consumedWine++;
    }
    if(currentOrder.cheese){
        sharedData->sharedStatistics.consumedCheese++;
    }
    if(currentOrder.salad){
        sharedData->sharedStatistics.consumedSalads++;
    }
}

int findChairInTable(shareDataSegment* sharedData, pid_t visitor, int tableIndex){
    for(int i = 0; i < 4; i++){
        if(sharedData->tables[tableIndex].chairs[i] == visitor){
            return i;
        }
    }
    return -1;

}

void sitInTheFirstEmptyChair(shareDataSegment* sharedData, pid_t visitor, int tableIndex){
    for(int i = 0; i < 4; i++){
        if(sharedData->tables[tableIndex].chairs[i] == -1){
            sharedData->tables[tableIndex].chairs[i] = visitor;
            sharedData->tables[tableIndex].chairsOccupied++;
             // if the table just became full update occupied value
            if(sharedData->tables[tableIndex].chairsOccupied == 4){
                sharedData->tables[tableIndex].isOccupied = true;
            }
            return;
        }
    }
}

void lastVisitorInformingOthers(shareDataSegment* sharedData, int emptyTableIndex){
    
    //table is empty now
    sharedData->tables[emptyTableIndex].isOccupied = false;
    sharedData->tables[emptyTableIndex].chairsOccupied = 0;
    for(int j = 0; j < 4; j++){
        sharedData->tables[emptyTableIndex].chairs[j] = -1;
    }

    int visitorsWaitingInBuffer = sharedData->fcfsWaitingBuffer.count;
    // if there are up to 4 visitora waiting in the buffer, wake them all up
    //and give them the table else wake up the first 4
    if(visitorsWaitingInBuffer > 4){
        visitorsWaitingInBuffer = 4;
    }

    // if bar is closing and there are no visitors waiting in the buffer and no orders to serve and all tables are empty
    //wake up the receptionist to close the bar. Otherwise the receptionist will be waiting for a visitor to serve(deadlock).
    if(sharedData->closingFlag && 
        sharedData->fcfsWaitingBuffer.count == 0 &&
        sharedData->orderBuffer.count == 0 &&
        sharedData->tables[0].isOccupied == false && sharedData->tables[1].isOccupied == false &&
        sharedData->tables[2].isOccupied == false){
            sem_post(&(sharedData->receptionistSem));
    }

    for(int i = 0; i < visitorsWaitingInBuffer; i++){
        // awake visitors and give them a chair in the empty table

        sem_post(&(sharedData->fcfsWaitingBuffer.positionSem[sharedData->fcfsWaitingBuffer.front]));

        // updating front (wrap around)
        sharedData->fcfsWaitingBuffer.front = (sharedData->fcfsWaitingBuffer.front + 1) % MAX_VISITORS;
        sharedData->fcfsWaitingBuffer.count--;
        // more positions are free in the buffer
        sem_post(&(sharedData->exceedingVisitorsSem));
    }


}

void presentStatistics(shareDataSegment* sharedData){
    fprintf(stdout, "\n\t\t\tStatistics of the day for ~ Bar in Nemea ~\n\n\n");
    fprintf(stdout, "Total visitors served: %d\n\n", sharedData->sharedStatistics.visitorsServed);
    fprintf(stdout, "Water consumed: %d\n", sharedData->sharedStatistics.consumedWater);
    fprintf(stdout, "Wine consumed: %d\n", sharedData->sharedStatistics.consumedWine);
    fprintf(stdout, "Cheese consumed: %d\n", sharedData->sharedStatistics.consumedCheese);
    fprintf(stdout, "Salads consumed: %d\n\n", sharedData->sharedStatistics.consumedSalads);

    if(sharedData->sharedStatistics.visitorsServed == 0){
        fprintf(stdout, "Average waiting time: 0 seconds\n");
        fprintf(stdout, "Average stay time: 0 seconds\n\n");
    }
    else{
        fprintf(stdout, "Average waiting time: %.5f seconds\n", sharedData->sharedStatistics.totalWaitingTime/sharedData->sharedStatistics.visitorsServed);
        fprintf(stdout, "Average stay time: %.5f seconds\n\n", sharedData->sharedStatistics.totalStayTime/sharedData->sharedStatistics.visitorsServed);
    }

}


int closingTheBar(shareDataSegment* sharedData, char* sharedMemoryName){
    //destroying semaphores
    int returnValue;

    for(int i = 0; i < MAX_VISITORS; i++){
        returnValue = sem_destroy(&(sharedData->fcfsWaitingBuffer.positionSem[i]));
        if(returnValue == -1){
            perror("sem_destroy failed");
            return -1;
        }
    }

    returnValue = sem_destroy(&(sharedData->exceedingVisitorsSem));
    if(returnValue == -1){
        perror("sem_destroy failed");
        return -1;
    }

    for(int i = 0; i < 12; i++){
        returnValue = sem_destroy(&(sharedData->orderBuffer.chairSem[i]));
        if(returnValue == -1){
            perror("sem_destroy failed");
            return -1;
        }
    }
    returnValue = sem_destroy(&(sharedData->mutex));
    if(returnValue == -1){
        perror("sem_destroy failed");
        return -1;
    }

    returnValue = sem_destroy(&(sharedData->receptionistSem));
    if(returnValue == -1){
        perror("sem_destroy failed");
        return -1;
    }

    //destroying shared memory
    returnValue = shm_unlink(sharedMemoryName);
    if(returnValue == -1){
        perror("shm_unlink failed");
        return -1;
    }

    return 0;
}