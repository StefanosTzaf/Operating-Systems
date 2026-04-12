#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <utils.h>
#include <sys/wait.h>
#include <getopt.h>

#define FORKED_VISITORS 45

int main(int argc, char *argv[]) {
    
    if(argc != 9){
        fprintf(stderr, "Usage: ./initializer -d <orderTime> -r <restTime> -s <sharedMemoryName> -l <logFileName.txt>\n");
        exit(EXIT_FAILURE);
    }
    int option;
    int maxOrderTime;
    int maxRestTime;
    char sharedMemoryName[64];
    char logFileName[64];

    while((option = getopt(argc, argv, "d:s:r:l:")) != -1){
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
        else if(option == 'r'){
            maxRestTime = atoi(optarg);
            if(maxRestTime <= 0){
                fprintf(stderr, "Give valid number for restTime please\n");
                exit(EXIT_FAILURE);
            }
        }
        else if(option == 'l'){
            snprintf(logFileName, sizeof(logFileName), "%s", optarg);
        }
        else{
            fprintf(stderr, "Usage: ./initializer -d <orderTime> -r <restTime> -s <sharedMemoryName> -l <logFileName.txt>\n");
            exit(EXIT_FAILURE);
        }
    }

    // open log file and create if not exists
    int logFd = open(logFileName, O_CREAT | O_RDWR | O_TRUNC, 0666);
    if (logFd == -1) {
        perror("log file open failed");
        exit(EXIT_FAILURE);
    }

    char* logFileCreation = " -- Log file for BAR IN NEMEA --\n\n\n\0";
    write(logFd, logFileCreation, strlen(logFileCreation));
    // close the log file , visitors and receptionist will write to this file and open by themselves
    close(logFd);

    int shmFd;
    size_t sharedMemorySize = sizeof(shareDataSegment);
    shareDataSegment* sharedData;

    // open shared memory
    shmFd = shm_open(sharedMemoryName, O_CREAT | O_RDWR, 0666);
    if (shmFd == -1) {
        perror("shared memory open failed");
        exit(EXIT_FAILURE);
    }

    // define the size
    if (ftruncate(shmFd, sharedMemorySize) == -1) {
        perror("ftruncate failed");
        exit(EXIT_FAILURE);
    }

    // Map shared memory in current address space
    sharedData = mmap(0, sharedMemorySize, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
    if (sharedData == MAP_FAILED) {
        perror("mmap failed");
        exit(EXIT_FAILURE);
    }
    
   
    initializeSharedValues(sharedData);
    

    pid_t pid = fork();
    pid_t receptionistPid;

    char orderTimeStr[16];
    char restTimeStr[16];
    snprintf(orderTimeStr, sizeof(orderTimeStr), "%d", maxOrderTime);
    snprintf(restTimeStr, sizeof(restTimeStr), "%d", maxRestTime);

    if (pid == -1) {
        perror("Error forking receptionist process");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0) {
        execlp("./receptionist", "./receptionist", "-d", orderTimeStr, "-s", sharedMemoryName, "-l", logFileName , NULL);
        exit(EXIT_FAILURE);
    }
    else{
        receptionistPid = pid;
    }


    pid_t visitorsPids[FORKED_VISITORS];

    for(int i = 0; i < FORKED_VISITORS; i++) {
        pid_t currentPid = fork();
        if (currentPid == -1) {
            perror("Error forking visitor process");
            exit(EXIT_FAILURE);
        }
        else if (currentPid == 0) {
            execlp("./visitor", "./visitor", "-d", restTimeStr, "-s", sharedMemoryName, "-l", logFileName, NULL);
            exit(EXIT_FAILURE);
        }
        else{
            visitorsPids[i] = currentPid;
        }
    }

    // wait for all children to finish
    int status;
    if(waitpid(receptionistPid, &status, 0) == -1){
        perror("Error waiting for receptionist");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < FORKED_VISITORS; i++) {
        if(waitpid(visitorsPids[i], &status, 0) == -1){
            perror("Error waiting for visitor");
            exit(EXIT_FAILURE);
        }
    }


    munmap(sharedData, sharedMemorySize);
    close(shmFd);
    
    exit(EXIT_SUCCESS);
}
