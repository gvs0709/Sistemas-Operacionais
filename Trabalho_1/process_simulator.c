/*----------*----------*----------*----------*----------*----------*
 *
 *  OPERATIONAL SYSTEMS     - ASSIGNMENT 01   - 25/08/2018
 *  VALÉRIA MENEZES BASTOS  - DCC/UFRJ
 *
 *  -------------------------------------
 *  <==> PROCESS SCHEDULER SIMULATOR <==>
 *  -------------------------------------
 * 
 *  PROGRAMMERS & ARCHITECTS
 *  ---------------------
 *  >>>>> Gabriel Villares Silveira         - 114089936
 *  >>>>> Hugo Kenupp Cunha Guimarães       - 109062709
 *  >>>>> Eduardo Carneiro                  - 113149505
 * 
 *  -------------------------------------
 *  <==> WHAT THIS PROGRAM DOES <==>
 *  -------------------------------------
 * 
 *  This program simulates the process administraion infrastructure of an operational system. A SHODDY OS. BUT STILL, AN OS.
 * 
 *  One function - maybe a thread? - constantly creates new processes, at random intervals, with random parameters, and sends them to the
 *      scheduler to handle, up to a maximum, pre-defined number, because our program needs an endpoint, after all.
 * 
 *----------*----------*----------*----------*----------*----------*/

/*
 *  PRELIMINARY COMMENT
 *  ---------------------
 *  I don't think we even need to deal with malloc and free in all this, since we do have a fixed, maximum number of processes to handle. We
 *      can just initialize the enitre array all at once, and use a simple counter variable to keep track of which process slot is the next in
 *      line for initialization/last process initialized. It's not enough memory space to need to free terminated processes, and I think it's
 *      cool we keep all of them around to present a final tally at the end of execution.
 * 
 *  I was thinking of using another struct for the process I/O requests, since the time of request and the code are associated 1-to-1. Two
 *      arrays are useful, too, if you don't want to deal with a struct inside a struct - accessing fields like that can be complicated,
 *      especially when you have to remember the disction of component access modes: '->' if accessing a component of a pointer to a struct,
 *      and '.' if you're referencing it directly.
 */

/*
 *  TO DO LIST
 *  ---------------------
 *  - Windows Thread Compatibility (windows.h or pthread for windows)
 *  - install libsodium on Windows
 *  <add here>
 */

/*
 *  INCLUDE LIBRARIES
 *  ---------------------
 *  <1> <stdio.h>       Standard I/O library, for printf.
 *  <2> <stdlib.h>      Standard general library, for malloc and free.
 *  <3> <string.h>      Useful String functions, like strcomp, and all that fun stuff!
 *  <4> <pthread.h>     Standard Thread library.
 *  <5> <sodium.h>      Secure random characters or integers library.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sodium.h>
#include <time.h>
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

/*
 *  DEFINES
 *  ---------------------
 *  <1> TIMESLICE       INT             Processor Timeslice             Size of the processor's timeslice. Expressed in abstract, generic TUs
 *                                                                          (Time Units).
 *  <2> MAX_PROCESSES   INT             Maximum Process Count           Maximum number of processes available for scheduling and execution,
 *                                                                          since our program needs an end-point.
 *  <3> MAX_IOREQUESTS  INT             Maximum I/O Request Count       Maximum number of I/O requests, per process.
 *  <4> T_DISC          INT             Disc Time                       Access time for the hard drive disk.
 *  <5> T_TAPE          INT             Tape Time                       Access time for the tape drive.
 *  <6> T_PRINTER       INT             Printer Time                    Access time for the printer.
 *  <7> DISC            CHAR            Disc Code                       Interrupt code for the disc drive access call.
 *  <8> TAPE            CHAR            Tape Code                       Interrupt code for the tape drive access call.
 *  <9> PRINTER         CHAR            Printer Code                    Interrupt code for the printer call.
 *  <10> NTHREADS       INT             Number of Threads               Number of threads to be created.
 *  <11> T              INT             Sleep time                      Time in seconds that threads will sleep.
 *  <12> MAX_PRIORITY   INT             Top priority                    The greatest priority a process can have (priority goes from 0 to 4)
 *  <13> IO_TYPE        INT             Maximum I/O types               Defines how many I/O types are exists in the simulator
 *  <14> MIN_SERVICE_TIME INT           Minimal process time            Minimal amount of time a process will spend using the processor. Expressed in abstract, generic TUs
 *  <15> MAX_SERVICE_TIME INT           Maximum process time            Maximum amount of time a process will spend using the processor. Expressed in abstract, generic TUs
 */

#define TIMESLICE 3
#define MAX_PROCESSES 10 // Temporarily reduced to test things out
#define MAX_IOREQUESTS 4 // It have to be 1 less than MIN_SERVICE_TIME
#define T_DISC 10
#define T_TAPE 20
#define T_PRINTER 30
#define DISC 'd'
#define TAPE 't'
#define PRINTER 'p'
#define NTHREADS  1
#define T 3
#define MAX_PRIORITY 5
#define IO_TYPE 3
#define MIN_SERVICE_TIME 5
#define MAX_SERVICE_TIME 26

/*
 *  PROCESS CONTROL BLOCK
 *  ---------------------
 *  Main data structure for process administration, created and used by the operational system - one for each process.
 *  Accessed and/or modified by most OS utilities, holds all relevant information about a process they could ever need to do their jobs.
 * 
 *  <1> PID             INT             Process Identifier              A unique number, used to tell processes from one another.
 *  <2> PPID            INT             Parent Process Identifier       Process Identifier (PID) of the process from which the instance was
 *                      -               -                                   created,  if any.
 *  <3> PRIORITY        INT             Process Priority Level          Execution priority indicator, used by the process scheduler, to 
 *                      -               -                                   determine who gets preference when selecting the next process for 
 *                      -               -                                   execution.
 *  <4> STATUS          INT             Process Execution Status        Execution status indicator, used by the process scheduler, to move 
 *                      -               -                                   the processes around the different execution queues, and handle 
 *                      -               -                                   them properly.
 *  <5> P_TIME          INT             Processor Execution Time        Remaining processor time to process completion, used by the simulator,
 *                      -               -                                   to determine when a process has finished, so as to be properly
 *                      -               -                                   removed from the queues and terminated. Expressed in abstract,
 *                      -               -                                   generic TUs (Time Units.)
 *  <6> E_TIME          INT             Total Execution Time            Total execution time of a process, from arrival to termination. 
 *                      -               -                                   Expressed in abstract, generic TUs (Time Units).
 *  <7> IOITERATOR      INT             I/O List Iterator               Auxiliary variable to keep track of the next I/O request.
 *  <8> IOTIME          INT[10]         I/O Request time list           List containing the trigger execution times of the process's I/O
 *                      -               -                                   requests.
 *  <9> IOLIST          CHAR[10]        I/O Request list                List containing the interrupt codes for the process's I/O requests.
 *  <10>S_TIME          INT             Process execution time          Estimated execution time of a process
 *  //
 *  // UNCOMMENT THE APPRPRIATE BLOCK TO USE THIS ONE.
 *  //
 *  <?> IOLIST          IOR[10]         I/O Request list                List containing the interrupt codes and the triger execution times
 *                                                                          for the process's I/O requests
 */

typedef struct process_pcb { //typedef struct process_pcb PCB;
    int PID, PPID, PRIORITY, STATUS, P_TIME, E_TIME, S_TIME, IOITERATOR, IOTIME[MAX_IOREQUESTS];
    char IOLIST[MAX_IOREQUESTS];
    // IOR IOLIST[MAX_IOREQUESTS];
}PCB;

/*
 *  PROCESS I/O REQUEST BLOCK
 *  ---------------------
 *  Holds the information about process I/O requests.
 *  Maybe I should move that IO list iterator in here as well, from the PCB? Makes sense, I guess.
 * 
 *  <1> IOTIME          INT[10]
 *  <2> IOLIST          CHAR[10]
 */

/*typedef struct process_ior { //typedef struct process_ior IOR;
    int IOTIME[10];
    char IOLIST[10];
}IOR;*/

/*
 *  GLOBAL VARABLES
 *  ---------------------
 *  <1> pid_counter     UNSIGNED INT    Process Identifier Counter      Handy variable to keep track of available PID numbers for assignment.
 *                      -               -                                   Holds the next open PID. Used by the Assemble_PCB function.
 *  <2> boostrapper     PCB *           PCB Initializer pointer         Anchor pointer from which the PCB structs are initialized, because
 *                      -               -                                   you don't return local variables in a function call. Used by the
 *                      -               -                                   Assemble_PCB function.
 *  <3> process_list    PCB *[]         Process List Array              Main container array for the PCBs.
 *  <4> high_queue      PCB *[]         High-Priority Process Queue     Queue that receives new processes, and those which just finished a
 *                                                                          tape or printer I/O operation. This queue must be finished before
 *                                                                          the Low-priority queue is processed.
 *  <5> low_queue       PCB *[]         Low-Priority Process Queue      Queue that receives returning, preempted processes, or those that 
 *                                                                          just finished a disc operation.
 *  <6> disc_queue      PCB *[]         Disc I/O Request Queue          Queue for the processes that requested disc access.
 *  <7> tape_queue      PCB *[]         Tape I/O Request Queue          Queue for the processes that requested tape access.
 *  <8> printer_queue   PCB *[]         Printer I/O Request Queue       Queue for the processes that requested printer access.
 *  <9> select_IO       UNSIGNED INT    Stores the time of the IO types Array with 3 positions, one for each I/O type time.
 */

unsigned int pid_counter = 100, select_IO[IO_TYPE];
PCB *bootstrapper, *process_list[MAX_PROCESSES],
    *high_queue[MAX_PROCESSES], *low_queue[MAX_PROCESSES],
    *disc_queue[MAX_PROCESSES], *tape_queue[MAX_PROCESSES],*printer_queue[MAX_PROCESSES];
    /**greatest_priority;*/
clock_t start_t, end_t, total_t;

/*
 *  FUNCTION: Assemble_PCB - PCB *
 *  ---------------------
 *  Properly initializes the PCB data structure - memmory allocation and all.
 *  THE ONLY SPOT IN THE PROGRAM WHERE MALLOC IS CALLED BESIDES IN THE THREAD ARGS ALLOCATION IN MAIN.
 */

// Fuck me, C doesn't support default function values.
// Defaults would be: pid = pid_counter +1, ppid = 0, priority = 0, status = 0, iterator = (0 or MAX_IOREQUESTS?).

PCB * Assemble_PCB(int pid, int ppid, int priority, int status, int s_time, int iterator) {
    bootstrapper = malloc(sizeof(PCB));
    bootstrapper->PID = pid;
    bootstrapper->PPID = ppid;
    bootstrapper->PRIORITY = priority;
    bootstrapper->STATUS = status;
    bootstrapper->S_TIME = s_time;
    bootstrapper->IOITERATOR = iterator;

    int aux = randombytes_uniform(MAX_IOREQUESTS+1);

    // To uncomment this section MAX_IOREQUESTS must be MAX_SERVICE_TIME - 1
    /*if (bootstrapper->S_TIME < MAX_IOREQUESTS){
        aux = randombytes_uniform(s_time + 1); // Executes the 'for' bellow a random number of times (ranges from 0 to S_TIME times)
    }

    else{
        aux = randombytes_uniform(MAX_IOREQUESTS + 1); // Executes the 'for' bellow a random number of times (ranges from 0 to MAX_IOREQUESTS times)
    }*/

    for (int i = 0; i < aux; ++i) {
        bootstrapper->IOTIME[i] = randombytes_uniform((const uint32_t) bootstrapper->S_TIME - 1) + 1; // Ranges from 1 to S_TIME-1

        if (i == 0){
            printf("| pid: %d, IOTIME[%d]: %d, ", pid, i, bootstrapper->IOTIME[i]);
        }

        else{
            for (int j = 0; j < i; ++j) {
                if (bootstrapper->IOTIME[i] == bootstrapper->IOTIME[j]){
                    bootstrapper->IOTIME[i] = randombytes_uniform((const uint32_t) bootstrapper->S_TIME - 1) + 1;
                    j = -1; // Checks IOTIME from the start again
                }
            }

            printf("| pid: %d, IOTIME[%d]: %d, ", pid, i, bootstrapper->IOTIME[i]);
        }

        switch (select_IO[randombytes_uniform(IO_TYPE)]){
            case T_DISC:
                bootstrapper->IOLIST[i] = DISC;
                break;

            case T_TAPE:
                bootstrapper->IOLIST[i] = TAPE;
                break;

            case T_PRINTER:
                bootstrapper->IOLIST[i] = PRINTER;
                break;

            default:
                printf("==> Something went wrong with select_IO: {");

                for (int j = 0; j < IO_TYPE; ++j){
                    if (j != (IO_TYPE - 1)) {
                        printf("%d, ", select_IO[j]);
                    }

                    else{
                        printf("%d}\n", select_IO[j]);
                        printf("\n");
                    }
                }

                exit(-1);
                //break;
        }

        printf("IOLIST[%d]: %c |\n", i, bootstrapper->IOLIST[i]);
    }

    printf("\n");
    // How to initialize P_TIME and E_TIME?

    return bootstrapper;
}

/*
 *  FUNCTION: Terminate     - void
 *  ---------------------
 *  Frees all allocated space.
 *  THE ONLY SPOT IN THE PROGRAM WHERE FREE IS CALLED.
 */

void Terminate() {
    for (int i = 0; i < MAX_PROCESSES; i++){
        free(process_list[i]); // I removed the '&' before process_list as it was giving a 'invalid pointer' error or something like that, not sure if its still free correctly thought
    }
}

/*
 *  FUNCTION: Create_Process     - void
 *  ---------------------
 *  Thread function responsable to create MAX_PROCESSES processes with random time between each one.
 */

void *Create_Process(void *arg){
    //int idThread = *(int *) arg;
    clock_t thread_time;
    double aux;

    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (i == 0){ // Don't wait to create the first processes
            printf("*------------------------------First process created------------------------------*\n");

            process_list[i] = Assemble_PCB(pid_counter, getppid(), 0, 0, randombytes_uniform(MAX_SERVICE_TIME) + MIN_SERVICE_TIME, 0);

            printf(" process_list[%d] = {pid: %d, ppid: %d, priority: %d, status: %d, service time: %d, IOiterator: %d}\n", i, process_list[i]->PID, process_list[i]->PPID, process_list[i]->PRIORITY, process_list[i]->STATUS, process_list[i]->S_TIME, process_list[i]->IOITERATOR);
            printf("\n");

            high_queue[i] = process_list[i];
            thread_time = clock();
            aux = (thread_time - start_t) * 1000. / CLOCKS_PER_SEC;

            printf(" Process %d arrived in high_queue[%d] at %6.3f + %6.3f\n", high_queue[i]->PID, i, (start_t * 1000. / CLOCKS_PER_SEC), aux);
            printf("*--------------------------------------------------------------------------------*");
            printf("\n");

            pid_counter++;
        }

        else{
            unsigned int time = randombytes_uniform(T); // randombytes_uniform() will be a random number between 0 and T, excluding T

            //sleep:
            #ifdef _WIN32
            Sleep(time);
            #else
            sleep(time);
            #endif

            process_list[i] = Assemble_PCB(pid_counter, getppid(), 0, 0, randombytes_uniform(MAX_SERVICE_TIME) + MIN_SERVICE_TIME, 0);

            printf(" process_list[%d] = {pid: %d, ppid: %d, priority: %d, status: %d, service time: %d, IOiterator: %d}\n", i, process_list[i]->PID, process_list[i]->PPID, process_list[i]->PRIORITY, process_list[i]->STATUS, process_list[i]->S_TIME, process_list[i]->IOITERATOR);
            printf("\n");

            high_queue[i] = process_list[i];
            thread_time = clock();
            aux = (thread_time - start_t) * 1000. / CLOCKS_PER_SEC;

            printf(" Process %d arrived in high_queue[%d] at %6.3f + %6.3f\n", high_queue[i]->PID, i, (start_t * 1000. / CLOCKS_PER_SEC), aux);
            printf("*--------------------------------------------------------------------------------*");
            printf("\n");

            pid_counter++;
        }
    }

    free(arg);
    pthread_exit(NULL);
}

/*
 *  FUNCTION: MAIN          - int
 *  ---------------------
 *  Program entry point: where the fun begins.
 */

int main(int argc, char const *argv[]) {
    start_t = clock();

    printf("==> Simulator start time: %6.3f\n", (start_t * 1000. / CLOCKS_PER_SEC));
    printf("\n");

    if (sodium_init() < 0) {
        printf("Panic! The Sodium library couldn't be initialized, it is not safe to use");
        return 1;
    }

    select_IO[0] = T_DISC;
    select_IO[1] = T_TAPE;
    select_IO[2] = T_PRINTER;

    pthread_t tid_sistema[NTHREADS];
    int *arg, t;

    for(t=0; t<NTHREADS; t++) {
        //printf("--Aloca e preenche argumentos para thread %d\n", t);
        arg = malloc(sizeof(int));

        if (arg == NULL) {
            printf("--ERROR: malloc()\n"); exit(-1);
        }

        *arg = t;

        if (pthread_create(&tid_sistema[t], NULL, Create_Process, (void*) arg)) {
            printf("--ERROR: pthread_create()\n"); exit(-1);
        }
    }

    for (t = 0; t < NTHREADS; t++) {
        if (pthread_join(tid_sistema[t], NULL)) {
            printf("--ERROR: pthread_join() \n"); exit(-1);
        }
    }

    Terminate();
    pthread_exit(NULL);
}
