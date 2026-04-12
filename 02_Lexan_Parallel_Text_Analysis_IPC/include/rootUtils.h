#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include "Set.h"


char* printingFdsToString(int numOfBuilders, int pipesSplitterToBuilder[][2]);

Set rootReadFromPipe(int readEnd);

void destroyMapNode(Pointer node);

int compareSetNodes(Pointer a, Pointer b);

void destroySetNode(Pointer node);

void printingTopK(Set set, int k, char* , char* inputfile);

void splitterCompleted(int signum);
void builderCompleted(int signum);
