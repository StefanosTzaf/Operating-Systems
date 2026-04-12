#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include "Map.h"
#include "builderUtils.h"   
#include <signal.h>

int main(int argc, char* argv[]){
   if(argc != 4){
      fprintf(stderr, "Usage: ./builder, <readEndForBuilder>, <writeEndForBuilder>, <totalWordsToRead>\n");
      exit(1);
   }

   int readEndFd = atoi(argv[1]);
   int writeEndFd = atoi(argv[2]);
   int hashSize = atoi(argv[3]);
   char buffer[4096];

   Map wordHashTable = mapCreate(builderCompareWords, destroyMapNode, hashSize);

   int sizeofWord = 0;
   int capacity = 10;
   char* word = malloc(capacity);

   while (1) {

      int bytes = read(readEndFd, buffer, sizeof(buffer));

      if (bytes <= 0) {
         break;
      }

      for(int i = 0; i < bytes; i++){


         if(buffer[i] == '-' && sizeofWord > 0){
            word[sizeofWord] = '\0';
            //if the word arrives first time in the builder add it to  the hash table
            if(mapFind(wordHashTable, word) == NULL){
               char* newWord = malloc(sizeofWord + 1);
               int* occurenceCounter = malloc(sizeof(int));
               *occurenceCounter = 1;
               strcpy(newWord, word);
               mapInsert(wordHashTable, newWord, occurenceCounter);
            }
            //else increment the occurenceCounter of the word
            else{
               int* occurenceCounter = mapFind(wordHashTable, word);
               (*occurenceCounter)++;
            }
            memset(word, '\0', sizeofWord);
            sizeofWord = 0;
         }
         else{
            sizeofWord++;

            if (sizeofWord >= capacity){
               capacity *= 2;
               word = realloc(word, capacity);
            }

            word[sizeofWord - 1] = buffer[i];
         }
      }
   }

   free(word);
   close(readEndFd);


   for(MapNode node = mapFirst(wordHashTable); node != NULL; node = mapGetNext(wordHashTable, node)){

      int sizeOfKey = strlen(mapNodeKey(node));
      char wordToPrint[sizeOfKey];
      strcpy(wordToPrint, mapNodeKey(node));

      int occurenceCounter;
      occurenceCounter = *(int*)mapNodeValue(node);
      //convert occurenceCounter to string
      char occurenceStr[countDigits(occurenceCounter)];
      sprintf(occurenceStr, "%d", occurenceCounter);

      //a buffer to store a word like this: word*5-  (5 is the occurenceCounter of the word "*" and "-" to seperate occurenceCounter and real words)
      int sizeOfBuffer = sizeOfKey + 3 + countDigits(occurenceCounter);
      char bufferToWrite[sizeOfBuffer];

      strcpy(bufferToWrite, wordToPrint);
      bufferToWrite[sizeOfKey] = '*';
      strcpy(bufferToWrite + sizeOfKey + 1, occurenceStr);
      bufferToWrite[sizeOfBuffer - 2] = '-';
      bufferToWrite[sizeOfBuffer - 1] = '\0';

      write(writeEndFd, bufferToWrite, strlen(bufferToWrite));
   }

   mapDestroy(wordHashTable);
   close(writeEndFd);
   //parent process pid
   pid_t rootPid = getppid();
   //sending singal to root
   kill(rootPid, SIGUSR2);
   
   exit(0);
}
