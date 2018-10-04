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
#include <stdbool.h>

#endif

/*
 *  DEFINES
 *  ---------------------
 *  <1> TIMESLICE       INT             Processor Timeslice             Size of the processor's timeslice (Quantum). Expressed in abstract, generic TUs
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
#define NTHREADS  2
#define T 3
#define MAX_PRIORITY 3
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
 *  <10>AUX             INT             Auxiliary variable              To simplify the simulator
 *  //
 *  // UNCOMMENT THE APPRPRIATE BLOCK TO USE THIS ONE.
 *  //
 *  <?> IOLIST          IOR[10]         I/O Request list                List containing the interrupt codes and the triger execution times
 *                                                                          for the process's I/O requests
 */

typedef struct process_pcb {
    int PID, PPID, PRIORITY, STATUS, P_TIME, E_TIME, IOITERATOR, IOTIME[MAX_IOREQUESTS], AUX, OLD_ITERATOR;
    char IOLIST[MAX_IOREQUESTS];
    bool PENDINGIO;
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

/*typedef struct process_ior {
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

unsigned int pid_counter = 100, select_IO[IO_TYPE], process_created = 0, processes_processed = 0, process_terminated = 0, HQ_Count = 0, LQ_Count = 0, DQ_Count = 0, TQ_Count = 0, PQ_Count = 0, CPU_Count = 0;
PCB *bootstrapper, *process_list[MAX_PROCESSES],
    *high_queue[MAX_PROCESSES], *low_queue[MAX_PROCESSES],
    *disc_queue[MAX_PROCESSES], *tape_queue[MAX_PROCESSES],*printer_queue[MAX_PROCESSES], *exited_cpu[MAX_PROCESSES];
clock_t start_t, end_t;
double total_t;
struct timespec tim, tim2;
//pthread_mutex_t exited_cpu_mutex;
bool cpu_running = false, disc_running = false, tape_running = false, printer_running = false, Round_Robin = false;


/*
 *  FUNCTION: Assemble_PCB - PCB *
 *  ---------------------
 *  Properly initializes the PCB data structure - memmory allocation and all.
 *  THE ONLY SPOT IN THE PROGRAM WHERE MALLOC IS CALLED BESIDES IN THE THREAD ARGS ALLOCATION IN MAIN.
 */

// Fuck me, C doesn't support default function values.
// Defaults would be: pid = pid_counter +1, ppid = 0, priority = 0, status = 0, iterator = 0.

PCB *Assemble_PCB(int pid, int ppid, int priority, int status, int p_time, int iterator){
    int aux = randombytes_uniform(MAX_IOREQUESTS+1); // Decides how many I/O requests will be made randomly, varies from 0 to MAX_IOREQUESTS - 1

    bootstrapper = malloc(sizeof(PCB));
    bootstrapper->PID = pid;
    bootstrapper->PPID = ppid;
    bootstrapper->PRIORITY = priority;
    bootstrapper->STATUS = status;
    bootstrapper->P_TIME = p_time;
    bootstrapper->IOITERATOR = iterator;
    bootstrapper->OLD_ITERATOR = iterator - 1; // Initialize OLD_ITERATOR as -1
    bootstrapper->AUX = aux; // Stores the number of I/O requests

    // To uncomment this section MAX_IOREQUESTS must be MAX_SERVICE_TIME - 1
    /*if (bootstrapper->P_TIME < MAX_IOREQUESTS){
        aux = randombytes_uniform(p_time + 1); // Executes the 'for' bellow a random number of times (ranges from 0 to P_TIME times)
    }

    else{
        aux = randombytes_uniform(MAX_IOREQUESTS + 1); // Executes the 'for' bellow a random number of times (ranges from 0 to MAX_IOREQUESTS times)
    }*/

    for (int i = 0; i < aux; ++i) {
        bootstrapper->IOTIME[i] = randombytes_uniform((const uint32_t) bootstrapper->P_TIME - 1) + 1; // Ranges from 1 to P_TIME-1

        if (i == 0){
            printf("| pid: %d, IOTIME[%d]: %d, ", pid, i, bootstrapper->IOTIME[i]);
        }

        else{
            for (int j = 0; j < i; ++j) {
                if (bootstrapper->IOTIME[i] == bootstrapper->IOTIME[j]){
                    bootstrapper->IOTIME[i] = randombytes_uniform((const uint32_t) bootstrapper->P_TIME - 1) + 1;
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

    if (aux == 0){
        bootstrapper->PENDINGIO = false;
        bootstrapper->IOITERATOR = -1;

        printf("| IOITERATOR = %d |\n", bootstrapper->IOITERATOR);
        printf("\n");
    }

    if (aux > 0){
        bootstrapper->PENDINGIO =true;
    }

    if (aux > 1){
        bootstrapper->PENDINGIO =true;
        //printf("IOTIME ordenation:\n");

        for (int k = 0; k < aux; ++k){
            //printf("| pid: %d, IOTIME[%d]: %d |\n", pid, k, bootstrapper->IOTIME[k]);

            if (bootstrapper->IOTIME[k] > bootstrapper->IOTIME[bootstrapper->IOITERATOR]){
                bootstrapper->IOITERATOR = k;
            }
        }

        printf("| IOITERATOR = %d |\n", bootstrapper->IOITERATOR);
        printf("\n");
    }

    // How to initialize E_TIME?

    return bootstrapper;
}

//---------------Round Robin waiting time/ turnaround time-------------------------------
/*void findWaitingTime(int P[], int n,int bt[], int wt[], int quantum){ // parameters: Process list, max_processes, burst time array, wait time array, timeslice
    int rem_bt[n];

    for (int i = 0 ; i < n ; i++) {
        rem_bt[i] = bt[i];
    }

    int t = 0;

    while (1){
        int done = 1;

        for (int i = 0 ; i < n; i++){
            if (rem_bt[i] > 0) {
                done = 0;

                if (rem_bt[i] > quantum){
                    t += quantum;
                    rem_bt[i] -= quantum;
                }

                else{
                    t = t + rem_bt[i];
                    wt[i] = t – bt[i];
                    rem_bt[i] = 0;
                }
            }
        }

        if (done == 1) {
            break;
        }
    }
}
void findTurnAroundTime(int P[], int n,int bt[], int wt[], int tat[]){ // parametres: process list, max processes, burst time array, wait time array, turnaraound time array
    for (int i = 0; i < n ; i++){
        tat[i] = bt[i] + wt[i];
    }


}

void findavgTime(int P[], int n, int bt[],int quantum){ // parametres: process list, max processes, burst time array, timeslice
    int wt[n], tat[n], total_wt = 0, total_tat = 0;
    findWaitingTime(P, n, bt, wt, quantum);
    findTurnAroundTime(P, n, bt, wt, tat);

    for (int i=0; i<n; i++){
        total_wt = total_wt + wt[i];
        total_tat = total_tat + tat[i];
    }

    printf("Average waiting time = %f", (float)total_wt / (float)n);
    printf("\nAverage turn around time =%f ", (float)total_tat / (float)n);
}
//-----------------//--------------------------------*/

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
    clock_t thread_time, temp_time = 0;
    double aux;

    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (i == 0){ // Don't wait to create the first processes
            printf("*------------------------------First process created------------------------------*\n");

            process_list[i] = Assemble_PCB(pid_counter, 1, MAX_PRIORITY, 0, randombytes_uniform(MAX_SERVICE_TIME) + MIN_SERVICE_TIME, 0);
            process_created++;

            printf(" process_list[%d] = {pid: %d, ppid: %d, priority: %d, status: %d, service time: %d, IOiterator: %d}\n", i, process_list[i]->PID, process_list[i]->PPID, process_list[i]->PRIORITY, process_list[i]->STATUS, process_list[i]->P_TIME, process_list[i]->IOITERATOR);
            printf("\n");

            /*high_queue[i] = process_list[i];
            HQ_Count++;

            thread_time = clock();
            aux = (thread_time - start_t) * 1000. / CLOCKS_PER_SEC;

            printf(" Process %d arrived in high_queue[%d] at %6.3f + %6.3f\n", high_queue[i]->PID, i, (start_t * 1000. / CLOCKS_PER_SEC), aux);*/
            printf("*--------------------------------------------------------------------------------*");
            printf("\n");

            pid_counter++;
            //temp_time = thread_time;
        }

        else{
            unsigned int time = randombytes_uniform(T); // randombytes_uniform() will be a random number between 0 and T, excluding T

            //sleep:
            #ifdef _WIN32
            Sleep(time);
            #else
            sleep(time);
            #endif

            process_list[i] = Assemble_PCB(pid_counter, 1, MAX_PRIORITY, 0, randombytes_uniform(MAX_SERVICE_TIME) + MIN_SERVICE_TIME, 0);
            process_created++;

            printf(" process_list[%d] = {pid: %d, ppid: %d, priority: %d, status: %d, service time: %d, IOiterator: %d}\n", i, process_list[i]->PID, process_list[i]->PPID, process_list[i]->PRIORITY, process_list[i]->STATUS, process_list[i]->P_TIME, process_list[i]->IOITERATOR);
            printf("\n");

            /*high_queue[i] = process_list[i];
            HQ_Count++;

            thread_time = clock();
            aux = (thread_time - temp_time) * 1000. / CLOCKS_PER_SEC;

            printf(" Process %d arrived in high_queue[%d] at %6.3f + %6.3f\n", high_queue[i]->PID, i, (temp_time * 1000. / CLOCKS_PER_SEC), aux);*/
            printf("*--------------------------------------------------------------------------------*");
            printf("\n");

            pid_counter++;
            //temp_time = thread_time;
        }
    }

    free(arg);
    pthread_exit(NULL);
}

/*
 *  FUNCTION: CPU     - void
 *  ---------------------
 *  Function responsable to simulate the use of the CPU by a process, using timeslice and comunicating with the scheduler.
 */

void *CPU(void *arg){
    cpu_running = true;

    PCB *p = (PCB *) arg;
    int control = 0; // auxiliary variable to store i
    bool io = false;

    tim.tv_sec = 0;
    tim.tv_nsec = 100000000; // 0.1 seconds

    printf("\n");
    printf("--Process %d in the CPU\n", p->PID);
    printf("\n");

    for (int i = 0; i < TIMESLICE; ++i) {
        nanosleep(&tim , &tim2);
        p->P_TIME--;

        if(p->P_TIME){ // Checks if the process has finished
            if (p->PENDINGIO && p->IOTIME[p->IOITERATOR] == p->P_TIME){
                p->IOTIME[p->IOITERATOR] = 0;

                //sends p->IOLIST[p->IOITERATOR] to the scheduler
                p->OLD_ITERATOR = p->IOITERATOR;

                for (int k = 0; k < p->AUX; ++k){ // Find next IOTIME
                    if (p->IOTIME[k] > p->IOTIME[p->IOITERATOR]){
                        p->IOITERATOR = k;
                    }
                }

                i = TIMESLICE;
                control = i;
                io = true;

                if (p->IOTIME[p->IOITERATOR] == 0){
                    p->PENDINGIO = false;
                }
            }

            control++;
        }

        else{ // Process terminated
            i = TIMESLICE;
            control = i;
            control++; // Makes control the same as
            process_terminated++;
            break;
        }

    }

    if (control == TIMESLICE){
        p->STATUS = 2; // Process preempted
        exited_cpu[CPU_Count] = p;

        if (CPU_Count++ > MAX_PROCESSES) { // Acrescenta de +1. Se tiver passado, faz a volta.
            CPU_Count = 0;
            //Round_Robin = true;
        }

        printf("--Process %d preempted\n", p->PID);
        printf("\n");
    }

    else{ // control > TIMESLICE
        if (io){
            p->STATUS = 3; // I/O request
            exited_cpu[CPU_Count] = p;

            if (CPU_Count++ > MAX_PROCESSES) { // Acrescenta de +1. Se tiver passado, faz a volta.
                CPU_Count = 0;
                //Round_Robin == true;
            }

            printf("--Process %d requested I/O\n", p->PID);
            printf("\n");
        }

        else{ // io = false
            p->STATUS = 4; // Process terminated

            printf("--Process %d terminated\n", p->PID);
            printf("\n");
        }
    }

    printf("--CPU thread finished (%d)\n", p->PID);
    printf("\n");

    cpu_running = false;
    free(arg);
    pthread_exit(NULL);
}

void *Disk_Handler(void *arg){
    disc_running = true;
    PCB *p = (PCB *) arg;
    int aux = T_DISC;

    tim.tv_sec = 0;
    tim.tv_nsec = 100000000; // 0.1 seconds

    while (aux){
        nanosleep(&tim , &tim2);
        aux--;
    }

    p->STATUS = 0; // Not running
    disc_running = false;

    free(arg);
    pthread_exit(NULL);
}

void *Tape_Handler(void *arg){
    tape_running = true;
    PCB *p = (PCB *) arg;
    int aux = T_TAPE;

    tim.tv_sec = 0;
    tim.tv_nsec = 100000000;

    while (aux){
        nanosleep(&tim , &tim2);
        aux--;
    }

    p->STATUS = 0; // Not running
    tape_running = false;

    free(arg);
    pthread_exit(NULL);
}

void *Printer_Handler(void *arg){
    printer_running = true;
    PCB *p = (PCB *) arg;
    int aux = T_PRINTER;

    tim.tv_sec = 0;
    tim.tv_nsec = 100000000;

    while (aux){
        nanosleep(&tim , &tim2);
        aux--;
    }

    p->STATUS = 0; // Not running
    printer_running = false;

    free(arg);
    pthread_exit(NULL);
}

/*----------*----------*----------*----------*----------*----------*
 * Toying around in a new function to guarantee there's no collision.
 */

/*void *Prototype_Scheduler(void *arg){

    // Thse shouldn't be initialized here - they're supposed to follow the queue loaders.
    //unsigned int HQ_Count = 0, LQ_Count = 0, DQ_Count = 0, TQ_Count = 0, PQ_Count = 0;

    // These SHOULD be initialized here, however - they're the scheduler's internal activity counters: they keep track of WHERE the scheduler is in the round-robins.
    unsigned int HQ_Walker = 0, LQ_Walker = 0, DQ_Walker = 0, TQ_Walker = 0, PQ_Walker = 0;

    while (!process_created){} // Waits until the first process is created

    // While there are still processes to finish, patroll EVERY queue!
    while (process_terminated < MAX_PROCESSES){
        high_queue[HQ_Walker]->STATUS = 1; // Set status to running

        if (pthread_create(NULL, NULL, CPU, high_queue[HQ_Walker])) { // Passes a process to CPU
            printf("--ERROR: pthread_create()\n");
            exit(-1);
        }

        if (high_queue[HQ_Walker]->STATUS == 2){ // CHECAGEM DE PREEMPÇÃO DO CPU
            //Preempta o processo no cpu pro último slot da low_queue, passa o próximo na high_queue pro cpu
            //Tem um jeito de pegar diretamente quem tá na CPU?

            low_queue[LQ_Count] = high_queue[HQ_Walker]; // <Ponteiro do PCB do processo na cpu>;     // Preempta o processo pra low priority.

            if ( LQ_Count++ > MAX_PROCESSES ){                              // Acrescenta de +1. Se tiver passado, faz a volta.
                LQ_Count = 0;
            }

            //CPU( high_queue[HQ_Walker] );           // Passa o próximo na high_queue pro CPU.
            if ( HQ_Walker++ > MAX_PROCESSES ){     // Acrescenta de +1. Se tiver passado, faz a volta.
                HQ_Walker = 0;
            }

        }*/
        // FINISHED DISC QUEUE CHECK.
        // TAPE AND PRINTER ARE THE SAME, BUT DUMPS IN HIGH_QUEUE INSTEAD OF LOW.
        /*if( DQ_Walker > DQ_Count ){                 // If this ever happens, then the queue advanced to the point of going round: we must reach the end, and go round as well, continuing with the next IF, immediately after this one.
            while ( DQ_Walker > DQ_Count ){
                // Disc I/O processes got o low priority queue.
                low_queue[LQ_Count] = disc_finished_queue[DQ_Walker];
                disc_finished_queue[DQ_Walker] = NULL;

                if ( LQ_Count++ > MAX_PROCESSES ){  // Acrescenta de +1. Se tiver passado, faz a volta.
                    LQ_Count = 0;
                }

                if ( DQ_Walker++ > MAX_PROCESSES ){ // Acrescenta de +1. Se tiver passado, faz a volta.
                    DQ_Walker = 0;
                }
            }
        }
        if( DQ_Walker < DQ_Count ){                 // Catch up with the finished process queue.
            while ( DQ_Walker < DQ_Count ){
                // Disc I/O processes got o low priority queue.
                low_queue[LQ_Count] = disc_finished_queue[DQ_Walker];
                disc_finished_queue[DQ_Walker] = NULL;

                if ( LQ_Count++ > MAX_PROCESSES ){  // Acrescenta de +1. Se tiver passado, faz a volta.
                    LQ_Count = 0;
                }
            }
        }*/
        /* Queues:
        high_queue
        low_queue
        disc_finished_queue
        tape_finished_queue
        printer_finished_queue
        */
    /*}

    free(arg);
    pthread_exit(NULL);
}*/

/*
 * Toying around in a new function to guarantee there's no collision.
 *----------*----------*----------*----------*----------*----------*/

void *Scheduler(void *arg){
    //int idThread = *(int *) arg;

    // These SHOULD be initialized here, however - they're the scheduler's internal activity counters: they keep track of WHERE the scheduler is in the round-robins.
    unsigned int HQ_Walker = 0, LQ_Walker = 0, DQ_Walker = 0, TQ_Walker = 0, PQ_Walker = 0, CPU_Walker = 0;
    double aux;
    bool first = true, HQ_Round_Robin = false, LQ_Round_Robin = false, DQ_Round_Robin = false, TQ_Round_Robin = false, PQ_Round_Robin = false;
    pthread_t CPU_thread = NTHREADS + 1, Disk_Thread = NTHREADS + 2, Tape_Thread = NTHREADS + 3, Printer_Thread = NTHREADS + 4;
    clock_t thread_time;

    printf("==> Scheduler starting...\n");
    printf("\n");

    while (!process_created){} // Waits until the first process is created

    while (process_terminated < MAX_PROCESSES){
        if (processes_processed <= MAX_PROCESSES){ // If not all processes have been on the high_queue for the first time
            if (process_created < MAX_PROCESSES){
                while (HQ_Count > process_created - 1){} // Waits a new process be created
            }

            high_queue[HQ_Count] = process_list[processes_processed];
            processes_processed++;

            thread_time = clock(); // NOT DONE!!!
            aux = (thread_time - start_t) * 1000. / CLOCKS_PER_SEC;

            printf("> Process %d arrived in high_queue[%d] at %6.3f + %6.3f\n", high_queue[HQ_Count]->PID, HQ_Count, (start_t * 1000. / CLOCKS_PER_SEC), aux);
            printf("\n");

            if (HQ_Count++ > MAX_PROCESSES) { // Acrescenta de +1. Se tiver passado, faz a volta.
                HQ_Count = 0;
                //HQ_Round_Robin = true;
            }
        }

        //if(!HQ_Round_Robin) {
        if (HQ_Walker != HQ_Count && HQ_Walker < HQ_Count && high_queue[HQ_Walker]->STATUS == 0) { // Send a process to CPU
            //aux = high_queue[HQ_Walker]->PID; // Maybe this is useless
            //processes_processed++;

            if (first){
                first = false;
            }

            else {
                while (cpu_running) {} // Waits the process that is using to finish CPU
            }

            high_queue[HQ_Walker]->STATUS = 1; // Set status to running

            if (pthread_create(&CPU_thread, NULL, CPU, (void *) high_queue[HQ_Walker])) { // Passes a process to CPU
                printf("--ERROR: pthread_create()\n");
                exit(-1);
            }

            if (HQ_Walker++ > MAX_PROCESSES) { // Acrescenta de +1. Se tiver passado, faz a volta.
                HQ_Walker = 0;
                //HQ_Round_Robin = false;
            }
        }
        //}

        //if (HQ_Round_Robin){
        if (HQ_Walker != HQ_Count && HQ_Walker > HQ_Count && high_queue[HQ_Walker]->STATUS == 0) { // Send a process to CPU
            //aux = high_queue[HQ_Walker]->PID; // Maybe this is useless
            //processes_processed++;

            if (first){
                first = false;
            }

            else {
                while (cpu_running) {} // Waits the process that is using to finish CPU
            }

            high_queue[HQ_Walker]->STATUS = 1; // Set status to running

            if (pthread_create(&CPU_thread, NULL, CPU, (void *) high_queue[HQ_Walker])) { // Passes a process to CPU
                printf("--ERROR: pthread_create()\n");
                exit(-1);
            }

            if (HQ_Walker++ > MAX_PROCESSES) { // Acrescenta de +1. Se tiver passado, faz a volta.
                HQ_Walker = 0;
                HQ_Round_Robin = false;
            }
        }
        //}

        if(exited_cpu[CPU_Walker] != NULL/* && !Round_Robin*/) {
            if (CPU_Walker != CPU_Count && CPU_Walker < CPU_Count && exited_cpu[CPU_Walker]->STATUS == 2) { // CHECAGEM DE PREEMPÇÃO DO CPU
                //Preempta o processo no cpu pro último slot da low_queue, passa o próximo na high_queue pro cpu
                //Tem um jeito de pegar diretamente quem tá na CPU?

                //pthread_mutex_lock(&exited_cpu_mutex);
                low_queue[LQ_Count] = exited_cpu[CPU_Walker]; // <Ponteiro do PCB do processo na cpu>;     // Preempta o processo pra low priority.
                low_queue[LQ_Count]->PRIORITY = 0;

                if (CPU_Walker++ > MAX_PROCESSES) { // Acrescenta de +1. Se tiver passado, faz a volta.
                    CPU_Walker = 0;
                    //Round_Robin == false;
                }

                //pthread_mutex_unlock(&exited_cpu_mutex);

                printf("> Process %d arrived at low_queue[%d] with priority %d\n", low_queue[LQ_Count]->PID, LQ_Count, low_queue[LQ_Count]->PRIORITY);
                printf("\n");

                if (LQ_Count++ > MAX_PROCESSES) { // Acrescenta de +1. Se tiver passado, faz a volta.
                    LQ_Count = 0;
                }

                /*if (HQ_Walker++ > MAX_PROCESSES) { // Acrescenta de +1. Se tiver passado, faz a volta.
                    HQ_Walker = 0;
                }*/

                //while (HQ_Walker > HQ_Count) {} // Waits a new process arrive at high_queue --> this is wrong! Como não deixar HQ_Walker "passar" HQ_Count? (isso considerando walker a cabeça da fila e count o fim

                //CPU( high_queue[HQ_Walker] );           // Passa o próximo na high_queue pro CPU.
                /*high_queue[HQ_Walker]->STATUS = 1; // Set status to running --> do jeito que está parece que da ruim aqui eventualmente (bem rápido até)
                //aux2 = high_queue[HQ_Walker]->PID;

                if (pthread_create(&CPU_thread, NULL, CPU, (void *) high_queue[HQ_Walker])) { // Passes a process to CPU
                    printf("--ERROR: pthread_create()\n");
                    exit(-1);
                }*/

                /*if ( HQ_Walker++ > MAX_PROCESSES ){     // Acrescenta de +1. Se tiver passado, faz a volta.
                    HQ_Walker = 0;
                }*/

            }

            if (CPU_Walker != CPU_Count && CPU_Walker < CPU_Count && exited_cpu[CPU_Walker]->STATUS == 3){ // Checagem de pedido de E/S
                switch (exited_cpu[CPU_Walker]->IOLIST[exited_cpu[CPU_Walker]->OLD_ITERATOR]){
                    case 'd':
                        //pthread_mutex_lock(&exited_cpu_mutex);
                        disc_queue[DQ_Count] = exited_cpu[CPU_Walker];
                        disc_queue[DQ_Count]->PRIORITY = 1;

                        if (CPU_Walker++ > MAX_PROCESSES) { // Acrescenta de +1. Se tiver passado, faz a volta.
                            CPU_Walker = 0;
                            //Round_Robin = false;
                        }
                        //pthread_mutex_unlock(&exited_cpu_mutex);

                        printf("> Process %d arrived at disc_queue[%d] with priority %d\n", disc_queue[DQ_Count]->PID, DQ_Count, disc_queue[DQ_Count]->PRIORITY);
                        printf("\n");

                        if (DQ_Count++ > MAX_PROCESSES) { // Acrescenta de +1. Se tiver passado, faz a volta.
                            DQ_Count = 0;
                            //DQ_Round_Robin = true;
                        }

                        break;

                    case 't':
                        //pthread_mutex_lock(&exited_cpu_mutex);
                        tape_queue[TQ_Count] = exited_cpu[CPU_Walker];
                        tape_queue[TQ_Count]->PRIORITY = 2;

                        if (CPU_Walker++ > MAX_PROCESSES) { // Acrescenta de +1. Se tiver passado, faz a volta.
                            CPU_Walker = 0;
                            //Round_Robin = false;
                        }
                        //pthread_mutex_unlock(&exited_cpu_mutex);

                        printf("> Process %d arrived at tape_queue[%d] with priority %d\n", tape_queue[TQ_Count]->PID, TQ_Count, tape_queue[TQ_Count]->PRIORITY);
                        printf("\n");

                        if (TQ_Count++ > MAX_PROCESSES) { // Acrescenta de +1. Se tiver passado, faz a volta.
                            TQ_Count = 0;
                            //TQ_Round_Robin = true;
                        }

                        break;

                    case 'p':
                        //pthread_mutex_lock(&exited_cpu_mutex);
                        printer_queue[PQ_Count] = exited_cpu[CPU_Walker];
                        printer_queue[PQ_Count]->PRIORITY = 2;

                        if (CPU_Walker++ > MAX_PROCESSES) { // Acrescenta de +1. Se tiver passado, faz a volta.
                            CPU_Walker = 0;
                            //Round_Robin = false;
                        }
                        //pthread_mutex_unlock(&exited_cpu_mutex);

                        printf("> Process %d arrived at printer_queue[%d] with priority %d\n", printer_queue[PQ_Count]->PID, PQ_Count, printer_queue[PQ_Count]->PRIORITY);
                        printf("\n");

                        if (PQ_Count++ > MAX_PROCESSES) { // Acrescenta de +1. Se tiver passado, faz a volta.
                            PQ_Count = 0;
                            //PQ_Round_Robin = true;
                        }

                        break;
                }

            }
        }

        if(exited_cpu[CPU_Walker] != NULL/* && Round_Robin*/) {
            if (CPU_Walker != CPU_Count && CPU_Walker > CPU_Count && exited_cpu[CPU_Walker]->STATUS == 2) { // CHECAGEM DE PREEMPÇÃO DO CPU
                //Preempta o processo no cpu pro último slot da low_queue, passa o próximo na high_queue pro cpu
                //Tem um jeito de pegar diretamente quem tá na CPU?

                //pthread_mutex_lock(&exited_cpu_mutex);
                low_queue[LQ_Count] = exited_cpu[CPU_Walker]; // <Ponteiro do PCB do processo na cpu>;     // Preempta o processo pra low priority.
                low_queue[LQ_Count]->PRIORITY = 0;

                if (CPU_Walker++ > MAX_PROCESSES) { // Acrescenta de +1. Se tiver passado, faz a volta.
                    CPU_Walker = 0;
                    //Round_Robin == false;
                }

                //pthread_mutex_unlock(&exited_cpu_mutex);

                printf("> Process %d arrived at low_queue[%d] with priority %d\n", low_queue[LQ_Count]->PID, LQ_Count, low_queue[LQ_Count]->PRIORITY);
                printf("\n");

                if (LQ_Count++ > MAX_PROCESSES) { // Acrescenta de +1. Se tiver passado, faz a volta.
                    LQ_Count = 0;
                    //LQ_Round_Robin = false;
                }

                /*if (HQ_Walker++ > MAX_PROCESSES) { // Acrescenta de +1. Se tiver passado, faz a volta.
                    HQ_Walker = 0;
                }*/

                //while (HQ_Walker > HQ_Count) {} // Waits a new process arrive at high_queue --> this is wrong! Como não deixar HQ_Walker "passar" HQ_Count? (isso considerando walker a cabeça da fila e count o fim

                //CPU( high_queue[HQ_Walker] );           // Passa o próximo na high_queue pro CPU.
                /*high_queue[HQ_Walker]->STATUS = 1; // Set status to running --> do jeito que está parece que da ruim aqui eventualmente (bem rápido até)
                //aux2 = high_queue[HQ_Walker]->PID;

                if (pthread_create(&CPU_thread, NULL, CPU, (void *) high_queue[HQ_Walker])) { // Passes a process to CPU
                    printf("--ERROR: pthread_create()\n");
                    exit(-1);
                }*/

                /*if ( HQ_Walker++ > MAX_PROCESSES ){     // Acrescenta de +1. Se tiver passado, faz a volta.
                    HQ_Walker = 0;
                }*/

            }

            if (CPU_Walker != CPU_Count && CPU_Walker > CPU_Count && exited_cpu[CPU_Walker]->STATUS == 3){ // Checagem de pedido de E/S
                switch (exited_cpu[CPU_Walker]->IOLIST[exited_cpu[CPU_Walker]->OLD_ITERATOR]){
                    case 'd':
                        //pthread_mutex_lock(&exited_cpu_mutex);
                        disc_queue[DQ_Count] = exited_cpu[CPU_Walker];
                        disc_queue[DQ_Count]->PRIORITY = 1;

                        if (CPU_Walker++ > MAX_PROCESSES) { // Acrescenta de +1. Se tiver passado, faz a volta.
                            CPU_Walker = 0;
                            //Round_Robin = false;
                        }
                        //pthread_mutex_unlock(&exited_cpu_mutex);

                        printf("> Process %d arrived at disc_queue[%d] with priority %d\n", disc_queue[DQ_Count]->PID, DQ_Count, disc_queue[DQ_Count]->PRIORITY);
                        printf("\n");

                        if (DQ_Count++ > MAX_PROCESSES) { // Acrescenta de +1. Se tiver passado, faz a volta.
                            DQ_Count = 0;
                            //DQ_Round_Robin = true;
                        }

                        break;

                    case 't':
                        //pthread_mutex_lock(&exited_cpu_mutex);
                        tape_queue[TQ_Count] = exited_cpu[CPU_Walker];
                        tape_queue[TQ_Count]->PRIORITY = 2;

                        if (CPU_Walker++ > MAX_PROCESSES) { // Acrescenta de +1. Se tiver passado, faz a volta.
                            CPU_Walker = 0;
                            //Round_Robin = false;
                        }
                        //pthread_mutex_unlock(&exited_cpu_mutex);

                        printf("> Process %d arrived at tape_queue[%d] with priority %d\n", tape_queue[TQ_Count]->PID, TQ_Count, tape_queue[TQ_Count]->PRIORITY);
                        printf("\n");

                        if (TQ_Count++ > MAX_PROCESSES) { // Acrescenta de +1. Se tiver passado, faz a volta.
                            TQ_Count = 0;
                            //TQ_Round_Robin = true;
                        }

                        break;

                    case 'p':
                        //pthread_mutex_lock(&exited_cpu_mutex);
                        printer_queue[PQ_Count] = exited_cpu[CPU_Walker];
                        printer_queue[PQ_Count]->PRIORITY = 2;

                        if (CPU_Walker++ > MAX_PROCESSES) { // Acrescenta de +1. Se tiver passado, faz a volta.
                            CPU_Walker = 0;
                            //Round_Robin = false;
                        }
                        //pthread_mutex_unlock(&exited_cpu_mutex);

                        printf("> Process %d arrived at printer_queue[%d] with priority %d\n", printer_queue[PQ_Count]->PID, PQ_Count, printer_queue[PQ_Count]->PRIORITY);
                        printf("\n");

                        if (PQ_Count++ > MAX_PROCESSES) { // Acrescenta de +1. Se tiver passado, faz a volta.
                            PQ_Count = 0;
                            //PQ_Round_Robin = true;
                        }

                        break;
                }

            }
        }

        if (disc_queue[DQ_Walker] != NULL) {
            if (disc_queue[DQ_Walker]->STATUS == 3) {
                while (disc_running) {}

                if (pthread_create(&Disk_Thread, NULL, Disk_Handler,
                                   (void *) disc_queue[DQ_Walker])) { // Passes a process to CPU
                    printf("--ERROR: pthread_create()\n");
                    exit(-1);
                }
            }

            if (disc_queue[DQ_Walker]->STATUS == 0) {
                low_queue[LQ_Count] = disc_queue[DQ_Walker];

                printf("> Process %d arrived at low_queue[%d] with priority %d\n", low_queue[LQ_Count]->PID, LQ_Count, low_queue[LQ_Count]->PRIORITY);
                printf("\n");

                if (DQ_Walker++ > MAX_PROCESSES) { // Acrescenta de +1. Se tiver passado, faz a volta.
                    DQ_Walker = 0;
                    //Round_Robin = false;
                }
            }
        }

        if (tape_queue[TQ_Walker] !=NULL) {
            if (tape_queue[TQ_Walker]->STATUS == 3) {
                while (tape_running) {}

                if (pthread_create(&Tape_Thread, NULL, Tape_Handler,
                                   (void *) tape_queue[TQ_Walker])) { // Passes a process to CPU
                    printf("--ERROR: pthread_create()\n");
                    exit(-1);
                }
            }

            if (tape_queue[TQ_Walker]->STATUS == 0) {
                high_queue[HQ_Count] = tape_queue[TQ_Walker];

                printf("> Process %d arrived in high_queue[%d] with priority %d\n", high_queue[HQ_Count]->PID, HQ_Count, high_queue[HQ_Count]->PRIORITY);
                printf("\n");

                if (TQ_Walker++ > MAX_PROCESSES) { // Acrescenta de +1. Se tiver passado, faz a volta.
                    TQ_Walker = 0;
                    //Round_Robin = false;
                }
            }
        }

        if (printer_queue[PQ_Walker] != NULL){
            if (printer_queue[PQ_Walker]->STATUS == 3) {
                while (printer_running) {}

                if (pthread_create(&Printer_Thread, NULL, Printer_Handler,
                                   (void *) printer_queue[PQ_Walker])) { // Passes a process to CPU
                    printf("--ERROR: pthread_create()\n");
                    exit(-1);
                }
            }

            if (printer_queue[PQ_Walker]->STATUS == 0) {
                high_queue[HQ_Count] = printer_queue[PQ_Walker];

                printf("> Process %d arrived in high_queue[%d] with priority %d\n", high_queue[HQ_Count]->PID, HQ_Count, high_queue[HQ_Count]->PRIORITY);
                printf("\n");

                if (PQ_Walker++ > MAX_PROCESSES) { // Acrescenta de +1. Se tiver passado, faz a volta.
                    PQ_Walker = 0;
                    //Round_Robin = false;
                }
            }
        }

        /*if (process_list[aux - 100]->STATUS == 3){ // Process terminated
            printf("\n");
            printf("> Process %d finished execution\n", process_list[aux - 100]->PID);
        }*/

        //code

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
    //int P[20],burst_time[20],quantum,n; // Round Robin waiting time/ turnaround time variables/ parameters
    start_t = clock();

    printf("==> Simulator start time: %6.3f\n", (start_t * 1000. / CLOCKS_PER_SEC));
    printf("\n");

    if (sodium_init() < 0) {
        printf("Panic! The Sodium library couldn't be initialized, it is not safe to use");
        return 1;
    }

    for (int i = 0; i < MAX_PROCESSES; ++i) {
        high_queue[i] = NULL;
        low_queue[i] = NULL;
        disc_queue[i] = NULL;
        tape_queue[i] = NULL;
        printer_queue[i] = NULL;
        exited_cpu[i] = NULL;
    }

    select_IO[0] = T_DISC;
    select_IO[1] = T_TAPE;
    select_IO[2] = T_PRINTER;

    pthread_t tid_sistema[NTHREADS];
    int *arg;

    for(int t = 0; t < NTHREADS; t++) {
        //printf("--Aloca e preenche argumentos para thread %d\n", t);
        arg = malloc(sizeof(int));

        if (arg == NULL) {
            printf("--ERROR: malloc()\n");
            exit(-1);
        }

        *arg = t;

        if (t == 0) {
            if (pthread_create(&tid_sistema[t], NULL, Create_Process, (void *) arg)) {
                printf("--ERROR: pthread_create()\n");
                exit(-1);
            }
        }

        if (t == 1){
            if (pthread_create(&tid_sistema[t], NULL, Scheduler, (void *) arg)) {
                printf("--ERROR: pthread_create()\n");
                exit(-1);
            }
        }
    }

    /*printf("==> Chamei CPU!!!\n");
    CPU(high_queue[0]);
    printf("==> Cabou CPU\n");*/

    for (int t = 0; t < NTHREADS; t++) {
        if (pthread_join(tid_sistema[t], NULL)) {
            printf("--ERROR: pthread_join() \n");
            exit(-1);
        }
    }

    Terminate();
    end_t = clock();

    printf("\n");
    printf("==> Simulator end time: %6.3f\n", (end_t * 1000. / CLOCKS_PER_SEC));
    printf("\n");

    total_t = end_t - start_t;

    printf("\n");
    printf("==> Simulator total time: %6.3f s\n", (total_t * 1000. / CLOCKS_PER_SEC));
    printf("\n");

    pthread_exit(NULL);
}
