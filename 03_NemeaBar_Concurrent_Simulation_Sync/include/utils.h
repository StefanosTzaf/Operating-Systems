#include "shm_structs.h"

void initializeSharedValues(shareDataSegment *sharedData);

shareDataSegment* attachShm(char* sharedMemoryName, int* sharedFd);

int isAnyTableEmpty(shareDataSegment* sharedData);

menuOrder randomizeOrder(pid_t visitorID, int logFd);

void updateStatistics(shareDataSegment* sharedData, menuOrder currentOrder);

int findChairInTable(shareDataSegment* sharedData, pid_t visitor, int tableIndex);

void lastVisitorInformingOthers(shareDataSegment* sharedData, int emptyTableIndex);

void sitInTheFirstEmptyChair(shareDataSegment* sharedData, pid_t visitor, int tableIndex);

int closingTheBar(shareDataSegment* sharedData, char* sharedMemoryName);

void presentStatistics(shareDataSegment* sharedData);