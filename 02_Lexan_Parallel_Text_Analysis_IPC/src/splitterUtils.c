#include "splitterUtils.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

//a variation of the djb2 hash function that take the modulo of the hash with the number of builders
//because we want to send the word to the builder (so the hash value should be <= than the number of builders)
int splitterHashFunction(char *word, int numberOfBuilders){
	unsigned int hash = 5381;
	for (char* s = word; *s != '\0'; s++)
		hash = (hash << 5) + hash + *s;
	return (hash % numberOfBuilders);

}

int* writeFdsToInt(char* pipeWriteEnds, int numOfBuilders){
	int* fds = malloc(numOfBuilders * sizeof(int));
	char* token = strtok(pipeWriteEnds, " ");
	int i = 0;
	
	while(token != NULL){
		fds[i] = atoi(token);
		token = strtok(NULL, " ");
		i++;
	}
	return fds;
}

void freeSplitterMapNode(Pointer node){
	MapNode mapNode = (MapNode)node;
	//free the key while the value for this hash is the same pointer
	free(mapNodeKey(mapNode));
	free(mapNode);
}

Map exclusionHashTable(char* fileName){
	char buffer[4096];
    int bytesRead;
    int lineCount = 0;

	int fd = open(fileName, O_RDONLY);
	if(fd == -1){
		perror("Error opening file");
		exit(1);
	}

    // Read the file in chunks
    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
        for (int i = 0; i < bytesRead; i++) {
            if (buffer[i] == '\n') {
                lineCount++;
            }
        }
    }

	Map exclusionMap = mapCreate(splitterCompareWords, freeSplitterMapNode, lineCount);

	lseek(fd, 0, SEEK_SET);
	char ch;
	int sizeOfWord = 0;
	int capacity = 10;
	char* word = malloc(capacity);

	while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0){
		for(int i = 0; i < bytesRead; i++){

			ch = buffer[i];

			if(sizeOfWord == capacity){
				capacity *= 2;
				word = realloc(word, capacity);
			}
			//in - line  file
			if(ch == '\n'){
				//this word should not be freed because it is used as a key in the map
				char* newWord = malloc(sizeOfWord);
				strcpy(newWord, word);
				mapInsert(exclusionMap, newWord, newWord);
				memset(word, '\0', sizeOfWord);
				sizeOfWord = 0;
			}

			else{
				word[sizeOfWord] = ch;
				sizeOfWord++;
			}
		}
	}

	// Handle the last word
	if(sizeOfWord > 0){
		char* lastWord = malloc(sizeOfWord);
		strcpy(lastWord, word);
		mapInsert(exclusionMap, lastWord, lastWord);
	}

	free(word);
	close(fd);
	return exclusionMap;
}


//compare function for the map
int splitterCompareWords(Pointer a, Pointer b){
	MapNode nodeB = (MapNode)b;
	return strcmp(a, mapNodeKey(nodeB));
}