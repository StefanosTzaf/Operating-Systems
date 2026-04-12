#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/times.h>
#include <fcntl.h>
#include <string.h>
#include <getopt.h>
#include "rootUtils.h"
#include <signal.h>

//global counters for the signals
int usr1Counter = 0;
int usr2Counter = 0;


int main(int argc, char* argv[]) {

    double t1, t2, cpu_time;
    struct tms tb1, tb2;
    double ticspersec;
    //retrieves the number of clock ticks per second for your system and stores it in the variable ticspersec
    ticspersec = (double) sysconf (_SC_CLK_TCK);
    //captures the current time (in clock ticks)
    t1 = (double) times (&tb1) ;

    if(argc != 13){
        fprintf(stderr, "Usage: ./lexan -i <InputFile> -l <numOfSplitter> -m <numOfBuilders> -t <TopPopular> -e <ExclusoionListFile> -o <OutputFile>\n" );
        exit(1);
    }

    int option;
    char* inputFile = NULL;
    int numOfSplitter;
    int numOfBuilders;
    int topPopular;
    char* exclusionFile = NULL;
    char* outputFile = NULL;
    while((option = getopt(argc, argv, "i:l:m:t:e:o:")) != -1){

        if(option == 'i'){
            inputFile = optarg;
        }
        else if(option == 'l'){
            numOfSplitter = atoi(optarg);
        }
        else if(option == 'm'){
            numOfBuilders = atoi(optarg);
        }
        else if(option == 't'){
            topPopular = atoi(optarg);
        }
        else if(option == 'e'){
            exclusionFile = optarg;
        }
        else if(option == 'o'){
            outputFile = optarg;
        } 
        else{
            exit(1);
        }
    }


    int fdInput = open(inputFile, O_RDONLY);
    if(fdInput == -1){
        fprintf(stderr, "Error opening file %s\n", inputFile);
        exit(1);
    }


    int lines = 1;
    int totalBytesOfFIle = 0;
    int bytesRead;
    char buffer[4096];

    // calculate the number of lines in the file
    while ((bytesRead = read(fdInput, buffer, sizeof(buffer))) > 0) {
        totalBytesOfFIle += bytesRead;
        for (int i = 0; i < bytesRead; i++) {
            if (buffer[i] == '\n') {
                lines++;
            }
        }
    }

    //indicates which byte(number etc the 3456st.) is the byte that the line starts
    int* firstByteOfEachLine = malloc((lines + 1) * sizeof(int));
    firstByteOfEachLine[0] = 0;
    // lines are 1 based
    lines = 1;
    lseek(fdInput, 0, SEEK_SET);
    int bytesOfLine = 0;
    // count bytes per line to pass to each splitter 
    // so that it knows where to start reading from
    while( (bytesRead = read(fdInput, buffer, sizeof(buffer))) > 0){
        for (int i = 0; i < bytesRead; i++) {
            bytesOfLine += sizeof(buffer[i]); ;
            if (buffer[i] == '\n') {
                firstByteOfEachLine[lines] = bytesOfLine + firstByteOfEachLine[lines - 1];
                lines++;
                bytesOfLine = 0;
            }
        }
    }   

    // Close the file, each time the splitters open it so that 
    // we don't need to reposition the read pointer
    close(fdInput);    


    //creating pipes for communication from splitters to builders(one for every builder)
    int pipesSplitterToBuilder[numOfBuilders][2];
    for (int b = 0; b < numOfBuilders; b++) {
        if(pipe(pipesSplitterToBuilder[b]) == -1){
            perror("Error creating pipe");
            exit(1);
        }
    }

    //creating pipe for communication from builders to root
    int pipesBuilderToRoot[2];
    if(pipe(pipesBuilderToRoot) == -1){
        perror("Error creating pipe");
        exit(1);
    }

    //a struct to define the signal's behavior
    struct sigaction sa1;
    //signal handler
    sa1.sa_handler = splitterCompleted;
    //SA_RESTART means that if a system call was interrupted by the signal handler,
    // the system call will automatically be restarted, rather than returning an error with EINTR
    sa1.sa_flags = SA_RESTART;
    //sa_mask is a set of signals that should be blocked while the signal handler is executing.
    //with sigemptyset no other signals will be blocked or ignored    
    sigemptyset(&sa1.sa_mask);
    //This line installs the signal handler for SIGUSR1 by calling sigaction
    if (sigaction(SIGUSR1, &sa1, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    struct sigaction sa2;
    sa2.sa_handler = builderCompleted;
    sa2.sa_flags = SA_RESTART;
    sigemptyset(&sa2.sa_mask);

    if (sigaction(SIGUSR2, &sa2, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

//---------------------------------------------------------------------splitters---------------------------------------------------------------------

    // Create l splitters with lines/l lines each
    // except for the last one which might take a few more lines due to imperfect division
    int linesForSplitter = lines / numOfSplitter;
    pid_t splitterPids[numOfSplitter];

    char numberOfBuilders[16];
    sprintf(numberOfBuilders, "%d", numOfBuilders);

    //creating a string of write end file descriptors of pipes so as to pass it to every splitter
    //also doing it one time before fork, so that we don't need to do it for every splitter
    char* pipeWriteEnds = printingFdsToString(numOfBuilders, pipesSplitterToBuilder);

    for (int i = 0; i < numOfSplitter; i++) {
        pid_t pid = fork();
        
        if (pid < 0) {
            perror("Error forking splitter %d");
            exit(EXIT_FAILURE);
        }

        // splitter
        else if (pid == 0){
            //this pipe has no to do with the splitter
            close(pipesBuilderToRoot[0]);
            close(pipesBuilderToRoot[1]);

            for(int b = 0; b < numOfBuilders; b++){
                // Close the read end of the pipe in the splitter
                close(pipesSplitterToBuilder[b][0]);
            }

            //calculating the start and end line of the splitter
            int startLine = ( (i == 0) ? 1 : (i * linesForSplitter) + 1);

            // If it is the last line, the last splitter will take the remaining lines
            int endLine = (i == numOfSplitter - 1) ? lines : (i + 1) * linesForSplitter;

            char start[16];
            char end[16];
            sprintf(start, "%d", startLine);
            sprintf(end, "%d", endLine);
            
            // after how many bytes it will start reading
            int position = i * linesForSplitter;
            char firstByteForSplitter[32];
            sprintf(firstByteForSplitter, "%d", firstByteOfEachLine[position]);
            
            //list of aarguements and path of process
            execlp("./splitter", "./splitter", inputFile, start, end, firstByteForSplitter, numberOfBuilders, pipeWriteEnds, exclusionFile, NULL);

            perror("Error executing splitter");
            exit(EXIT_FAILURE);
        }
         
        else {
            // In the parent, we store the PID of the created splitter
            splitterPids[i] = pid;
        }
        
    }


//---------------------------------------------------------------------builders---------------------------------------------------------------------
    pid_t builderPids[numOfBuilders];
    for (int b = 0; b < numOfBuilders; b++) {
        pid_t pid = fork();
        
        if (pid == -1) {
            perror("Error forking builder process");
            exit(1);
        }

        //builder
        else if (pid == 0) {
            //close the read end of the pipe root - builder
            close(pipesBuilderToRoot[0]);
            // Close the read end of the pipe SPLITER - BUILDER in the builder exept for the pipe of this builder
            for(int i = 0; i < numOfBuilders; i++){
                if(i != b){
                    close(pipesSplitterToBuilder[i][0]);
                }
                // Close the write end of the pipe in the builder
                close(pipesSplitterToBuilder[i][1]);
            }

            int readEndForBuilder = pipesSplitterToBuilder[b][0];
            char readEndForBuilderStr[16];
            sprintf(readEndForBuilderStr, "%d", readEndForBuilder);

            int writeEndForBuilder = pipesBuilderToRoot[1];
            char writeEndForBuilderStr[16];
            sprintf(writeEndForBuilderStr, "%d", writeEndForBuilder);

            //how many bytes each builder will read (approximatelly)
            totalBytesOfFIle/=numOfBuilders; 
            //the average length of an english word is 5 so we divide the total bytes by 5
            //and thes are the number of words that each builder will read Approximatelly!
            totalBytesOfFIle/=5;
            char totalBytesStr[64];

            sprintf(totalBytesStr, "%d", totalBytesOfFIle);
            execlp("./builder", "./builder", readEndForBuilderStr, writeEndForBuilderStr, totalBytesStr, NULL);
            exit(EXIT_SUCCESS);
        }
        else {
            builderPids[b] = pid;
        }
    }

    //execution wii continue here only for the root process
    close(pipesBuilderToRoot[1]);
    for(int i = 0; i < numOfBuilders; i++){
        close(pipesSplitterToBuilder[i][0]);
        close(pipesSplitterToBuilder[i][1]);
    }   

    Set wordsWithOccurencyCounter = rootReadFromPipe(pipesBuilderToRoot[0]);

    // Wait for all the splitters and builders to finish and ensure that they have finished properly
    for (int i = 0; i < numOfSplitter; i++) {
        int status;
        if(waitpid(splitterPids[i], &status, 0) == -1){
            perror("Error waiting for splitter");
            exit(1);
        }
    }

    for (int i = 0; i < numOfBuilders; i++) {
        int status;
        if(waitpid(builderPids[i], &status, 0) == -1){
            perror("Error waiting for builder");
            exit(1);
        }
    }

    close(pipesBuilderToRoot[0]);

    printingTopK(wordsWithOccurencyCounter, topPopular, outputFile, inputFile);

    free(firstByteOfEachLine);
    free(pipeWriteEnds);
    setDestroy(wordsWithOccurencyCounter);

    fprintf(stdout,"Signal SIGUSR1 was received %d times\n", usr1Counter);
    fprintf(stdout,"Signal SIGUSR2 was received %d times\n", usr2Counter);

    t2 = (double)times(&tb2) ;
    cpu_time = (double)(( tb2 . tms_utime + tb2 . tms_stime ) - ( tb1 . tms_utime + tb1 . tms_stime ));
    fprintf (stdout,"Run time of root was %lf sec ( REAL time ) although we used the CPU for %lf sec ( CPU time ).\n", (t2 - t1) / ticspersec , cpu_time / ticspersec );

    exit(0);
}