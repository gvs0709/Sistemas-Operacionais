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
 *  >>>>> Gabriel Villares Silveira         - <dre>
 *  >>>>> Hugo Kenupp Cunha Guimarães       - 109062709
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
 *  INCLUDE LIBRARIES
 *  ---------------------
 *  <1> <stdio.h>       Standard I/O library, for printf.
 *  <2> <stdlib.h>      Standard general library, for malloc and free.
 *  <3> <string.h>      Useful String functions, like strcomp, and all that fun stuff!
 *  <4> <time.h>        rand() is our friend.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*
 *  DEFINES
 *  ---------------------
 *  <1> timeslice       INT             Processor Timeslice             Size of the processor's timeslice. Expressed in abstract, generic TUs
 *                                                                          (Time Units).
 *  <2> max_processes   INT             Maximum Process Count           Maximum number of processes available for scheduling and execution,
 *                                                                          since our program needs an end-point.
 *  <3> max_iorequests  INT             Maximum I/O Request Count       Maximum number of I/O requests, per process.
 *  <4> T_DISC          INT             Disc Time                       Access time for the hard drive disk.
 *  <5> T_TAPE          INT             Tape Time                       Access time for the tape drive.
 *  <6> T_PRINTER       INT             Printer Time                    Access time for the printer.
 *  <7> DISC            CHAR            Disc Code                       Interrupt code for the disc drive access call.
 *  <8> TAPE            CHAR            Tape Code                       Interrupt code for the tape drive access call.
 *  <9> PRINTER         CHAR            Printer Code                    Interrupt code for the printer call.
 */
#define timeslice 3;
#define max_processes 100;
#define max_iorequests 100;
#define T_DISC 10;
#define T_TAPE 20;
#define T_PRINTER 40;
#define DISC 'd';
#define TAPE 't';
#define PRINTER 'p';

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
 *  <?> IOLIST          IOR[10]         I/O Request list                List containing the interrupt codes and the triiger execution times
 *                                                                          for the process's I/O requests
 */

typedef struct process_pcb PCB;
//typedef struct process_ior IOR;

struct process_pcb
{
    int PID, PPID, PRIORITY, STATUS, P_TIME, E_TIME, IOITERATOR, IOTIME[10];
    char IOLIST[10];
    // IOR IOLIST[10];
};

/*
 *  PROCESS I/O REQUEST BLOCK
 *  ---------------------
 *  Holds the information about process I/O requests.
 *  Maybe I should move that IO list iterator in here as well, from the PCB? Makes sense, I guess.
 * 
 *  <1> IOTIME          INT[10]
 *  <2> IOLIST          CHAR[10]
 */

/*
struct process_ior
{
    int IOTIME[10];
    char IOLIST[10];
};
*/

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
PCB *bootstrapper, *process_list[ max_processes ],
    *high_queue[ max_processes ], *low_queue[ max_processes ]
    *disc_queue[ max_processes ], *tape_queue[ max_processes ],*printer_queue[ max_processes ];
//  vcode being a b**** and complaining about process_list definition, saying it "expected a']'", and accusing a phony error about an
//  undefined "ax_processes", because it's missing that m, for some reason. Ignore it, I suppose.

/*
 *  FUNCTION: Assemble_PCB - PCB *
 *  ---------------------
 *  Properly initializes the PCB data structure - memmory allocation and all.
 *  THE ONLY SPOT IN THE PROGRAM WHERE MALLOC IS CALLED.
 */

// Fuck me, C doesn't support default function values.
// Defaults would be: pid = pid_counter +1, ppid = 0, priority = 0, status = 0.

PCB * Assemble_PCB( int pid, int ppid, int priority, int status )
{
    // Should I use " 6*sizeof( int ) " instead? it's been a while since I've used C.
    bootstrapper = malloc( sizeof( PCB ) );
    bootstrapper = { pid, ppid, priority, status };
    /*
    AFAIK, this kind of struct initialization is valid in C, but vcode is complaninig about it, saying it "expected an expression"
        at the first '{', revise it.

    In any case, if it's in fact, wrong, junst uncomment this, I suppose:

    bootstrapper->PID = pid;
    bootstrapper->PPID = ppid;
    bootstrapper->PRIORITY = priority;
    bootstrapper->STATUS = status;

    */
    return bootstrapper;
};

/*
 *  FUNCTION: Terminate     - void
 *  ---------------------
 *  Frees all allocated space.
 *  THE ONLY SPOT IN THE PROGRAM WHERE FREE IS CALLED.
 */

void Terminate()
{
    for (int i = 0; i < max_processes; i++){
        free( &process_list[ i ] );
    }
}

/*
 *  FUNCTION: MAIN          - int
 *  ---------------------
 *  Program entry point: where the fun begins.
 */

int main(int argc, char const *argv[])
{
    /* code */

    Terminate();
    return 0;
}
