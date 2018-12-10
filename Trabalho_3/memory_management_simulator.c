#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sodium.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>

enum{
    NTHREADS = 1, // Number of threads
    MAX_FRAMES = 64, // Main memory's max number of frames
    MAX_VIRTUAL_PAGES = 64, // Process's max number of virtual pages
    PAGE_SIZE = 4, // Page size in KB
    MAX_PROCESSES = 10, // Maximum number of process that  will be created
    PROCESS_TIME = 3, // Both process creation interval and process time to page solicitation
    WORKING_SET_LIMIT = 4 // Maximum number of pages a process can have in main memory
};

typedef struct process_pcb{
    int PID, /*PPID, PRIORITY, STATUS, P_TIME, E_TIME, IOITERATOR, IOTIME[MAX_VIRTUAL_PAGES], AUX, OLD_ITERATOR, */VIRTUAL_PAGES, PAGE_NUMBER[MAX_VIRTUAL_PAGES], SIZE;
    //char IOLIST[MAX_VIRTUAL_PAGES];
    //bool PENDINGIO;
}PCB;

typedef struct frame{
    unsigned int SIZE, FRAME_ID, PROCESS_PID; // PROCESS_PID is the PID of the process with a page in the frame
    struct frame *left;
    struct frame *right;
}FRAME;

typedef struct main_memory{
    unsigned int SIZE, NFRAMES, FREE_SPACE;
    FRAME *FRAME_ROOT;
}MEMORY;

typedef struct swap_memory{
    int SIZE, FREE_SPACE;
}SWAP;

unsigned int pid_counter = 100;
PCB *bootstrapper, *process_list[MAX_PROCESSES];
MEMORY *mainMemory;
SWAP *swapMemory;

PCB *Assemble_PCB(int pid){
    //int aux = randombytes_uniform(MAX_IOREQUESTS+1); // Decides how many I/O requests will be made randomly, varies from 0 to MAX_IOREQUESTS - 1
    int aux;

    bootstrapper = malloc(sizeof(PCB));
    bootstrapper->PID = pid;
    /*bootstrapper->PPID = ppid;
    bootstrapper->PRIORITY = priority;
    bootstrapper->STATUS = status;
    bootstrapper->P_TIME = p_time;
    bootstrapper->IOITERATOR = iterator;
    bootstrapper->OLD_ITERATOR = iterator - 1; // Initialize OLD_ITERATOR as -1
    bootstrapper->AUX = aux; // Stores the number of I/O requests*/
    bootstrapper->VIRTUAL_PAGES = randombytes_uniform(MAX_VIRTUAL_PAGES) + 1; // Decides how many virtual pages the process will have. Varies from 1 to MAX_VIRTUAL_PAGES
    bootstrapper->SIZE = bootstrapper->VIRTUAL_PAGES * PAGE_SIZE;

    for(int j = 0; j < MAX_VIRTUAL_PAGES; j++){
        bootstrapper->PAGE_NUMBER[j] = 0;
    }

    for (int i = 0; i < bootstrapper->VIRTUAL_PAGES; i++) {
        aux = randombytes_uniform(MAX_VIRTUAL_PAGES);
        //bootstrapper->PAGE_NUMBER[i] = 1; //randombytes_uniform((const uint32_t) bootstrapper->P_TIME - 1) + 1; // Ranges from 1 to P_TIME-1

        if (bootstrapper->PAGE_NUMBER[aux] == 0){
            bootstrapper->PAGE_NUMBER[aux] = 1; // Creates a page with id aux, that ranges from 0 to 63
        }

        else{
            while(bootstrapper->PAGE_NUMBER[aux] != 0){
                aux = randombytes_uniform(MAX_VIRTUAL_PAGES);
            }

            bootstrapper->PAGE_NUMBER[aux] = 1;
        }

        /*switch (select_IO[randombytes_uniform(IO_TYPE)]){
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

        printf("IOLIST[%d]: %c |\n", i, bootstrapper->IOLIST[i]);*/
    }

    /*printf("\n");

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

    // How to initialize E_TIME?*/

    return bootstrapper;
}

unsigned int create_frame(MEMORY *caller, FRAME **leaf){
    unsigned int FRAME_ID = caller->NFRAMES + 1, SIZE = caller->SIZE;
    //void *SIZE = caller->SIZE;

    if(*leaf == NULL){
        *leaf = (FRAME*) malloc(sizeof(FRAME));
        (*leaf)->FRAME_ID = FRAME_ID;
        (*leaf)->SIZE = SIZE;
        (*leaf)->PROCESS_PID = 0; // The frame is empty

        /* initialize the children to null */
        (*leaf)->left = NULL;
        (*leaf)->right = NULL;
    }

    else if(FRAME_ID < (*leaf)->FRAME_ID){ // Descobrir como calcular o tamanho dos filhos esquerdo e direito!!!
        FRAME_ID += create_frame(caller, &(*leaf)->left);
    }

    else if(FRAME_ID > (*leaf)->FRAME_ID){
        FRAME_ID += create_frame(caller, &(*leaf)->right);
    }

    return FRAME_ID;
}

void initialize_memory(){
    mainMemory = malloc(sizeof(MEMORY));
    mainMemory->SIZE = MAX_VIRTUAL_PAGES * PAGE_SIZE; // Main memory size in KB. Default is 64 * 4 = 256 KB
    mainMemory->FRAME_ROOT = NULL;
    //mainMemory->FREE_SPACE = mainMemory->SIZE; // The memory is empty
    mainMemory->NFRAMES = 0;

    mainMemory->NFRAMES = create_frame(mainMemory, &mainMemory->FRAME_ROOT); // Creates the first frame occupying all the memory
}

void Terminate(){
    for (int i = 0; i < MAX_PROCESSES; i++){
        free(process_list[i]);
    }
}

int main(int argc, char const *argv[]){
    //int P[20],burst_time[20],quantum,n; // Round Robin waiting time/ turnaround time variables/ parameters
    //start_t = clock();

    //printf("==> Simulator start time: %6.3f\n", (start_t * 1000. / CLOCKS_PER_SEC));
    //printf("\n");

    if (sodium_init() < 0) {
        printf("Panic! The Sodium library couldn't be initialized, it is not safe to use");
        return 1;
    }

    /*for (int i = 0; i < MAX_PROCESSES; ++i) {
        high_queue[i] = NULL;
        low_queue[i] = NULL;
        disc_queue[i] = NULL;
        tape_queue[i] = NULL;
        printer_queue[i] = NULL;
        exited_cpu[i] = NULL;
    }

    select_IO[0] = T_DISC;
    select_IO[1] = T_TAPE;
    select_IO[2] = T_PRINTER;*/

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

        //if (t == 0) {
            if (pthread_create(&tid_sistema[t], NULL, Create_Process, (void *) arg)) {
                printf("--ERROR: pthread_create()\n");
                exit(-1);
            }
        //}

        /*if (t == 1){
            if (pthread_create(&tid_sistema[t], NULL, Scheduler, (void *) arg)) {
                printf("--ERROR: pthread_create()\n");
                exit(-1);
            }
        }*/
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
    /*end_t = clock();

    printf("\n");
    printf("==> Simulator end time: %6.3f\n", (end_t * 1000. / CLOCKS_PER_SEC));
    printf("\n");

    total_t = end_t - start_t;

    printf("\n");
    printf("==> Simulator total time: %6.3f s\n", (total_t * 1000. / CLOCKS_PER_SEC));
    printf("\n");*/

    pthread_exit(NULL);
}