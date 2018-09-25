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
 *  >>>>> Eduardo Carneiro                  - <dre>
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
 *  - Windows Thread Compatibility (windows.h or libsodium for windows)
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
 *  <12> MAX_PRIORITY   INT             Top priority                    The greatest priority a process can have plus 1 (priority goes from 0 to 5)
 */

#define TIMESLICE 3
#define MAX_PROCESSES 100
#define MAX_IOREQUESTS 100
#define T_DISC 10
#define T_TAPE 20
#define T_PRINTER 40
#define DISC 'd'
#define TAPE 't'
#define PRINTER 'p'
#define NTHREADS  1
#define T 3
#define MAX_PRIORITY 5

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
 *                      -               -                                   removed from the queues and termiated. Expressed in abstract,
 *                      -               -                                   generic TUs (Time Units.)
 *  <6> E_TIME          INT             Total Execution Time            Total execution time of a process, from arrival to termination. 
 *                      -               -                                   Expressed in abstract, generic TUs (Time Units).
 *  <7> IOITERATOR      INT             I/O List Iterator               Auxiliary variable to keep track of the next I/O request.
 *  <8> IOTIME          INT[10]         I/O Request time list           List containing the trigger execution times of the process's I/O
 *                      -               -                                   requests.
 *  <9> IOLIST          CHAR[10]        I/O Request list                List containing the interrupt codes for the process's I/O requests.
 *  //
 *  // UNCOMMENT THE APPRPRIATE BLOCK TO USE THIS ONE.
 *  //
 *  <?> IOLIST          IOR[10]         I/O Request list                List containing the interrupt codes and the triger execution times
 *                                                                          for the process's I/O requests
 */

typedef struct process_pcb { //typedef struct process_pcb PCB;
    int PID, PPID, PRIORITY, STATUS, P_TIME, E_TIME, IOITERATOR, IOTIME[10];
    char IOLIST[10];
    // IOR IOLIST[10];
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
 */

unsigned int pid_counter = 100;
PCB *bootstrapper, *process_list[MAX_PROCESSES],
    *high_queue[MAX_PROCESSES], *low_queue[MAX_PROCESSES],
    *disc_queue[MAX_PROCESSES], *tape_queue[MAX_PROCESSES],*printer_queue[MAX_PROCESSES],
    *greatest_priority;

/*
 *  FUNCTION: Assemble_PCB - PCB *
 *  ---------------------
 *  Properly initializes the PCB data structure - memmory allocation and all.
 *  THE ONLY SPOT IN THE PROGRAM WHERE MALLOC IS CALLED.
 */

// Fuck me, C doesn't support default function values.
// Defaults would be: pid = pid_counter +1, ppid = 0, priority = 0, status = 0.

PCB * Assemble_PCB( int pid, int ppid, int priority, int status ) {
    // Should I use " 6*sizeof( int ) " instead? it's been a while since I've used C.
    bootstrapper = malloc(sizeof(PCB));
    bootstrapper->PID = pid;
    bootstrapper->PPID = ppid;
    bootstrapper->PRIORITY = priority;
    bootstrapper->STATUS = status;

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
 *
 */

void *Create_Process(void *arg){
    //int idThread = *(int *) arg;

    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (i == 0){ // Don't wait to create the first processes
            process_list[i] = Assemble_PCB(pid_counter, 0, randombytes_uniform(MAX_PRIORITY), 0);

            printf("--First process created--\n");
            printf("process_list[%d] = {pid: %d, ppid: %d, priority: %d, status: %d}\n", i, process_list[i]->PID, process_list[i]->PPID, process_list[i]->PRIORITY, process_list[i]->STATUS);
            printf("\n");

            pid_counter++;
            greatest_priority = process_list[i];

            printf("---> greatest_priority->priority = %d\n", greatest_priority->PRIORITY);
            printf("\n");
        }

        else{
            //sleep:
            #ifdef _WIN32
            Sleep(randombytes_uniform(T));
            #else
            sleep(randombytes_uniform(T)); // randombytes_uniform() will be a random number between 0 and T, excluding T
            #endif

            process_list[i] = Assemble_PCB(pid_counter, 0, randombytes_uniform(MAX_PRIORITY), 0);

            printf("-> New process created!\n");
            printf("process_list[%d] = {pid: %d, ppid: %d, priority: %d, status: %d}\n", i, process_list[i]->PID, process_list[i]->PPID, process_list[i]->PRIORITY, process_list[i]->STATUS);
            printf("\n");

            pid_counter++;

            if (process_list[i]->PRIORITY > greatest_priority->PRIORITY){
                greatest_priority = process_list[i];

                printf("---> greatest_priority->priority = %d\n", greatest_priority->PRIORITY);
                printf("\n");
            }
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
    if (sodium_init() < 0) {
        /* panic! the library couldn't be initialized, it is not safe to use */
        printf("Panic! The Sodium library couldn't be initialized, it is not safe to use");
        return 1;
    }

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
    //return 0;
}
