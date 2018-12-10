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
    int PID, /*PPID, PRIORITY, STATUS, P_TIME, E_TIME, IOITERATOR, IOTIME[MAX_VIRTUAL_PAGES], AUX, OLD_ITERATOR, */VIRTUAL_PAGES, SIZE;
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

PCB *Assemble_PCB(int pid, int ppid, int priority, int status, int p_time, int iterator){
    //int aux = randombytes_uniform(MAX_IOREQUESTS+1); // Decides how many I/O requests will be made randomly, varies from 0 to MAX_IOREQUESTS - 1

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

    // To uncomment this section MAX_IOREQUESTS must be MAX_SERVICE_TIME - 1
    /*if (bootstrapper->P_TIME < MAX_IOREQUESTS){
        aux = randombytes_uniform(p_time + 1); // Executes the 'for' bellow a random number of times (ranges from 0 to P_TIME times)
    }

    else{
        aux = randombytes_uniform(MAX_IOREQUESTS + 1); // Executes the 'for' bellow a random number of times (ranges from 0 to MAX_IOREQUESTS times)
    }*/

    /*for (int i = 0; i < aux; ++i) {
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
    }*/

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
    unsigned int FRAME_ID = caller->NFRAMES + 1, SIZE = caller->FREE_SPACE;

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
    mainMemory->FREE_SPACE = mainMemory->SIZE; // The memory is empty
    mainMemory->NFRAMES = 0;

    mainMemory->NFRAMES = create_frame(mainMemory, &mainMemory->FRAME_ROOT); // Creates the first frame occupying all the memory
}