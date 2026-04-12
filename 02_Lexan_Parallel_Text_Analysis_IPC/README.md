Files and organization (Modularity):

First of all there are 3 different source code files that represent three different type of processes :
1) lexan -root.c source file- process (creating the other processes and pipes and introducing the results ).
2) splitter process (seperating the words from the lines that are responsible for and send them to a builder via pipes)
3) builder process (keeping words' occurenceCounter counter in hash tables and send them to the root, when they will not have anything to read)

For every type of process a source code "utils" file has been made , that includes help functions for each process respectively. 


Functions :

Lexan :
    Begining with the lexan process and its root.c source code file, after analyzing the options given
from the user in command line it counts the lines of the file to calculate how many lines each splitter
has to process. The last splitter may get a bit more lines due to the non imperfect division, also it 
calculates the exact byte that each splitter should start reading - otherwise each splitter should read 
the file from its start and in this way the task will be repeated many times. Then , it creates n pipes 
between builders and splitters (n the number of builders) and one pipe for builders - root.
    After that it creates with fork , the processes for builders and splitters and call exec* syscall in
the child process after ensuring that all useless file descriptors are closed. Also it closes in the parent
proscess the useless file descritors and then it starts reading from the readend of pipe that builders write.
Then it waits until every process terminates properly and presents the results. Using a set to keep the results
coming from builders not only do they ensure that the results would be sorted , so the printing process would be 
easier (via a setLast and a setPrevious function see Set.c) but also root does not have to wait until all the
builders end to start sorting the data , it can do it concurrently. We are sure that root will read every byte that
builders print to pipe because read as a blocking syscall will return 0 only when all writeEnds of the pipe are 
closed (so the builders would have finished). A the end it writes the results (words frequency and occurenceCounter)
in output file , after calculating the frequency of each "interesting" word (freq = WordOccurenceCounter / TotalNumberOfInterestingWordsInText)

Splitter :
    After being called from root , and analyzing the arguements passing from exec* , it starts reading from 
the file. It seperates the words if it finds whitespace or punctional character. Numbers are not taking
into account. One letter words -- like s or t coming from isn't and Bob's -- are excluded as well as
the words into the exclusion hash table (for o(1) search). Also capital and lowercase letter are considered
the same, for example word and WoRd are the same words at all.  

Builders : 
    They read from their pipes and storing the words in a hash table(its size calculated by the approximation
of the words that each builder will store (multiplying by 2 in the map create function to keep loading factor low)
it will read). If a word is found more than once its occurenceCounter is increased. After finishing reading , builder
starts writing data into a pipe with the root in a format that root will be able to distinguish occurenceCounters and words.


    Also, for signals i have define two global variables to count the count of received signals and i have define
the handler functions in rootUtils.c. In root i have used the struct sigaction sa1, to define the signals'
behavior (for example read and write not to being stopped by signal).

At the end i calculate the real and cpu time of root and i print them on command prompt (as well as signals received).
Top-k words are printed in the output file given.
