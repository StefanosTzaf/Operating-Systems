#include "Map.h"

int splitterHashFunction(char *word, int numberOfBuilders);

int* writeFdsToInt(char* pipeWriteEnds, int numOfBuilders);

Map exclusionHashTable(char* fileName);

void freeSplitterMapNode(Pointer node);

int splitterCompareWords(Pointer a, Pointer b);
