#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "splitterUtils.h"
#include <signal.h>

int main(int argc, char* argv[]){
    // without arguments (the file is open from the exec call, we do not pass it as an argument)
    if(argc != 8){
        fprintf(stderr, "Usage: ./splitter, <inputFile>, <startLine>, <endLine>, <firstByteForSplitter>, <numberOfBuilders>, <pipeWriteEnds>, <exclusionFile>\n");
        exit(1);
    }

    int fd = open(argv[1], O_RDONLY);
    if(fd == -1){
        perror("Error opening file");
        exit(1);
    }
    
    // read the file from where this specific splitter starts
    int startLine = atoi(argv[2]);
    int endLine = atoi(argv[3]);
    int firstByteToRead = atoi(argv[4]);
    int numberOfBuilders = atoi(argv[5]);
    char* writeEnds = argv[6];

    char* exclusionFile = argv[7];   
    Map exclusionMap = exclusionHashTable(exclusionFile);
    //file descriptors for writing in every pipe
    int* writeEndFds = writeFdsToInt(writeEnds, numberOfBuilders);
    
    // set the read pointer to the first byte of the splitter
    lseek(fd, firstByteToRead, SEEK_SET);
    int currentLine = startLine;


    char buffer[4096];
    bool allLinesRead = false;
    int bytesRead;

    int size = 0;
    //the initial capacity for words
    int capacity = 10;
    char *word = malloc(capacity * sizeof(char));

    while( (bytesRead = read(fd, buffer, sizeof(buffer))) > 0){

        //----------------------------------------------- words' seperation and checks -----------------------------------------------
        int i = 0;
        while(i < bytesRead){

            char ch = buffer[i];

            if (ch == '\n') {
                currentLine++;
            }
            // if we reached the next line, after the last one, the splitter's job is done
            if(currentLine > endLine){
                allLinesRead = true;
                // Handle the last word
                if(size > 1){
                    int hash = splitterHashFunction(word, numberOfBuilders);
                    if(mapFind(exclusionMap, word) == NULL){
                        word[size] = '-';
                        write(writeEndFds[hash], word, strlen(word));
                    }
                }
                // for the next word
                memset(word, '\0', capacity);
                size = 0;
                break;
            }
            
            // if the character is a letter, add it to the word
            if (isalpha(ch)) {
            // dynamic increase of word (we have no limit on the word size)
                if (size >= capacity - 1) {
                    capacity *= 2;
                    word = realloc(word, capacity * sizeof(char));
                }
                //to deem word GoodmorninG and goodmorning as the same word
                if(ch >= 'A' && ch <= 'Z'){
                    ch = tolower(ch);
                }
                word[size++] = ch;
            }

            else if (ispunct(ch)) {              
                // a punctuation character found and the word ends 
                if(size > 1){
                    int hash = splitterHashFunction(word, numberOfBuilders);
                    if(mapFind(exclusionMap, word) == NULL){
                        word[size] = '-';
                        write(writeEndFds[hash], word, strlen(word));
                    }
                }

                memset(word, '\0', capacity);
                size = 0;
                //else skip the punctuation character if it is in the beggining of the word(size = 0)
            }

            else if (isspace(ch) ) {
                if (size > 1 ) {
                    int hash = splitterHashFunction(word, numberOfBuilders);
                    if(mapFind(exclusionMap, word) == NULL){
                        word[size] = '-';
                        write(writeEndFds[hash], word, strlen(word));
                    }
                }
                memset(word, '\0', capacity);
                size = 0;
                    
            } 
            i++;
        }

        // if we have read the entire buffer and have not read
        // the lines corresponding to the splitter, then read again
        if(allLinesRead){
            break;
        }

    }
    free(word);

    close(fd);

    for(int i = 0; i < numberOfBuilders; i++){
        close(writeEndFds[i]);
    }
    free(writeEndFds);
    mapDestroy(exclusionMap);

    //parent process pid
    pid_t rootPid = getppid();
    //sending singal to root
    kill(rootPid, SIGUSR1);
    exit(0);
}