
WARNING: Programs are terminated ONLY by calling ./closer -s <sharedMemoryName> from another terminal, more
details in closing section below.

Command Line Execution :
    
    Instead of shmid is given the shared memory name because Posix shared memory instructions are used,
    as mentioned in Piazza.

 -- initializer : ./initializer -d <orderTime> -r <restTime> -s <sharedMemoryName> -l <logFileName.txt>
    Order time is parameter for receptionist and restTime for visitors. LogFile name is the name that
    we want to give to the file that logging facts will be stored to. If it does not exist it is created.
 
 -- receptionist: ./receptionist -d <orderTime> -s <sharedMemoryName> -l <logFileName.txt\n>
    sharedMemoryName and logFile name should be the same as in initializer! 
    ***For more convenience receptionist is created with fork and exec* inside initializer***
 
 -- visitor: ./visitor -d <restTime> -s <sharedMemoryName> -l <logFileName.txt\n>
    SharedMemoryName and logFile name should be the same as in initializer too.
    Some visitors are created in initializer but we are able to create more via command line.
 
 -- monitor: ./monitor -s sharedMemoryName
    it prints the current state of shared memory. Only called by command line.

DEFINES:
 
 -- In initializer is defined the value of visitors that are created by default and can be modified without 
    any problem. Visitors can also be created by another terminal so this is not the only choice to make new visitors.

 -- In shm_structs.h the number of FCFS buffer positions is defined, this can also be modified if we want more
    visitors to wait inside the bar with a more "fair" policy. If in a specific time there are more waiting
    to take a seat the exceeding would be suspended in exceeding semaphore.



Shared Memory:
    The main shared memory struct consists of sub-structs to enhance modularity and clean code.

    First sub-struct is about statistics.

    Second sub-struct is about circular waiting buffer that ensures FCFS policy for the first MAX_VISITORS
    (defined in the headerfile and easily modifiable). One semaphore for each position of the buffer ensures
    that only one process would be suspended there. The othe variable have to do with the positions of the buffer
    and wrap-around ability.
    If we did not have this buffer fcfs could not be assured while in one semafore are just suspended processes and wake
    up randomly. Now with one semaphore for every buffer position we know who came first.
    
    However this can not be defined dynammicaly (the shared memory segment has to ve defined before the execution). So there is
    an extra semaphore for exceeding visitors, initialized to MAX_VISITORS. Only when Fcfs buffer if full visitors will be suspended there
    (without fairness of fcfs in this case).

    Thex next sub-struct is about orders. For orders there is also a circular buffer to ensure that the first customer ordered
    will be served first. It has 12 positions, as many as the chairs (only visitors that have sat can order). These semaphores could have
    been located in table struct as well (one semaphore for each chair) but i prefered the first implementation. Also there is a struct
    inside ordersCircularBuffer that indicates what every order consists of (menuOrder)

    Also there are one semaphore for mutual exclusion and one for receptionist (receptioniist will be suspended there whenever
    there is no work to do).

    Finally, there is an array of struct table that stores the state of tables (occupied or not - which chairs etc).

    A bool variable indicates if the bar is closing or not.


Clarification:
   If a visitor has sat and the table has not been occupied by 4 and he leaves the bar, then the chair can be occupied by another visitor.
   As mentioned in the assignment, only when the table is full it should be freed from all the visitors that have sat there to be
   available again fo others.


Function of the bar:

    Everything starts with initializer call. After initializer is called, receptionist is made as mentioned. Also some visitors are
    made and executed. Receptionist at first is suspended in its semaphore while there is not work do. Whenever wakes up, checks if the 
    bar is about to close, otherwise is starting preparing orders with the with FCFS policy. If there is an order to serve updates 
    statistics and sleeps for a random time in the interval given. Then receptionist awakes the visitor sitting in the chair with the order
    just prepared.

    Visitor: Visitor is created by initializer or by the command line (both choices available). Visitor checks if bar is closing so as no
    to enter the bar. After that, decreases the exceeding buffer semapphore (if its value is less than 0 circular buffer is full and new visitors
    have to be suspended there till there is some space in buffer). Then visitor checks if the buffer is empty AND if there is a table 
    available to take a seat immediatelly. If not, visitor will be suspended in the buffer. When visitor takes a seat make an order and wait for it in 
    his chair semaphore after awakening receptionist. When visitor's order is ready and has been woken up by receptionist, visitor eats for a 
    random time in the interval given and then leaves the bar.
    
    IMPORTANT: Before leaving, if visitor was the last one of its table and table was occupied, then wakes up 4 visitors (if there are any) that are 
    waiting in the buffer to take a seat. Also if he is the last one in the bar and the bar is closing wakes up receptionist to close the bar.
    These two functions are important so as to avoid deadlocks.
   

Closing the bar:
   The bar is closing only when the closer is called and ALL the visitors that are inside the bar (eating or waiting order or to take a sit) 
   terminate properly. When closing flag is set to 1 visitors no longer enter the bar but the ones that are inside are NOT terminated by force.

   There are two time cases when the closer is invoked:
      - When the bar is closing and there are no visitors inside the bar. in this case closer HAVE to wake up the receptionist.
      - When the bar is closing and there are visitors inside the bar. In this case the last visitor wakes up the receptionist.

Clarifications for times:
 -- Average times are not stored in shared memory while they can be easily calculated whenever needed (by total visitors and total time).
 -- In average stay time is NOT included the time that visitor is waiting to take a seat. Only the time from when he takes a seat till he leaves the bar.


Monitors and logging: 

   Monitors can be called at any time by command line. They print the current state of shared memory. Logging file is created by initializer 
   and is used by all processes to log facts. The same logg file should given if a visitor is created by command line. Txt format is strongly 
   recommended for the log file (give as arguement log.txt not only log).