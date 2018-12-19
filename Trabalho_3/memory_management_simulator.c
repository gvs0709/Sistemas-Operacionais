#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sodium.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>

enum{
    NTHREADS = 2, // Number of threads
    MAX_FRAMES = 64, // Main memory's max number of frames
    MAX_VIRTUAL_PAGES = 64, // Process's max number of virtual pages
    PAGE_SIZE = 4, // Page size in KB
    MAX_PROCESSES = 10, // Maximum number of process that  will be created
    PROCESS_TIME = 3, // Both process creation interval and process time to page solicitation
    WORKING_SET_LIMIT = 4 // Maximum number of pages a process can have in main memory
};

typedef struct Page {
    unsigned int num, OWNER_PID; // OWNER_PID is the pid of the process that owns the page
    struct Page *next;
}PAGE;

typedef struct process_pcb{
    unsigned int PID, VIRTUAL_PAGES, PAGE_NUMBER[MAX_VIRTUAL_PAGES], SIZE, pages_in_mem;
    bool firstPage;
    PAGE *first, *last; // Linked list of process pages
}PCB;

typedef struct frame{
    unsigned int SIZE, FRAME_ID, PROCESS_PID; // PROCESS_PID is the PID of the process with a page in the frame
    int PAGE_ID;
    struct frame *left;
    struct frame *right;
    struct frame *next;
}FRAME;

typedef struct main_memory{
    unsigned int SIZE, NFRAMES, FREE_SPACE, PREVIOUS_FRAME;
    FRAME *FRAME_ROOT;
}MEMORY;

typedef struct swap_memory{
    int num_pag;
    PAGE *pag_atual;
    PAGE *pag_ultima;
}SWAP_MEMORY;

unsigned int pid_counter = 100;
PCB *bootstrapper, *process_list[MAX_PROCESSES];
MEMORY *mainMemory = NULL;
SWAP_MEMORY *swapMemory = NULL;

void AddNewPage(unsigned int value, PCB *process){
    PAGE *current = (PAGE*)malloc(sizeof(PAGE));
    current->num = value;
    current->next = NULL;
    current->OWNER_PID = process->PID;

    if(process->firstPage == true){
        process->first = current;
        process->last = process->first;
        process->firstPage = false;
    }

    else {
        process->last->next = current;
        process->last = current;
    }

    /*if(process->firstPage < process->VIRTUAL_PAGES) {
        process->firstPage++;
    }*/
}

FRAME *search_memory(int key, FRAME *leaf){ // Ainda adaptando!!!
    if(leaf != 0){
        if(key == leaf->SIZE){ // verificar se leaf->SIZE esta correto
            return leaf;
        }

        else if(key < leaf->SIZE){ // Talvez tenha q mudar o if else
            return search_memory(key, leaf->left);
        }

        else{
            return search_memory(key, leaf->right);
        }
    }

    else return 0;
}

FRAME *search_frame(int key, FRAME *leaf){ // Ainda adaptando!!!
    if(leaf != 0){
        if(key == leaf->FRAME_ID){ // verificar se leaf->SIZE esta correto
            return leaf;
        }

        else if(key < leaf->FRAME_ID){ // Talvez tenha q mudar o if else
            return search_memory(key, leaf->left);
        }

        else{
            return search_memory(key, leaf->right);
        }
    }

    else return 0;
}

PCB *Assemble_PCB(unsigned int pid){
    //int aux = randombytes_uniform(MAX_IOREQUESTS+1); // Decides how many I/O requests will be made randomly, varies from 0 to MAX_IOREQUESTS - 1
    unsigned int aux;
    PAGE *dummy;

    bootstrapper = malloc(sizeof(PCB));
    bootstrapper->PID = pid;
    bootstrapper->VIRTUAL_PAGES = randombytes_uniform(MAX_VIRTUAL_PAGES) + 1; // Decides how many virtual pages the process will have. Varies from 1 to MAX_VIRTUAL_PAGES
    bootstrapper->SIZE = bootstrapper->VIRTUAL_PAGES * PAGE_SIZE;
    bootstrapper->firstPage = true;
    bootstrapper->pages_in_mem = 0;

    for(int j = 0; j < MAX_VIRTUAL_PAGES; j++){
        bootstrapper->PAGE_NUMBER[j] = 0;
    }

    for (int i = 0; i < bootstrapper->VIRTUAL_PAGES; i++) {
        aux = randombytes_uniform(MAX_VIRTUAL_PAGES);

        if (bootstrapper->PAGE_NUMBER[aux] == 0){
            bootstrapper->PAGE_NUMBER[aux] = 1; // Creates a page with id aux, that ranges from 0 to 63
            AddNewPage(aux, bootstrapper);
        }

        else{
            while(bootstrapper->PAGE_NUMBER[aux] != 0){
                aux = randombytes_uniform(MAX_VIRTUAL_PAGES);
            }

            bootstrapper->PAGE_NUMBER[aux] = 1;
            AddNewPage(aux, bootstrapper);
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

    printf("Process Page List: {%d", bootstrapper->first->num);

    if(bootstrapper->first->next == NULL){
        printf("}\n");
        printf("\n");
    }

    else{
        printf(", ");
    }

    dummy = bootstrapper->first;

    while(dummy->next != NULL){
        dummy = dummy->next;

        if(dummy->next != NULL) {
            printf("%d, ", dummy->num);
        }

        else{
            printf("%d}\n", dummy->num);
        }
    }

    printf("\n");

    /*if (aux == 0){
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
    }*/

    return bootstrapper;
}

unsigned int create_frame(MEMORY *caller, FRAME **leaf){
    unsigned int FRAME_ID = caller->NFRAMES + 1, SIZE = caller->FREE_SPACE;

    if(*leaf == NULL){
        *leaf = (FRAME*) malloc(sizeof(FRAME));
        (*leaf)->FRAME_ID = FRAME_ID;
        (*leaf)->SIZE = SIZE;
        (*leaf)->PAGE_ID = -1;
        (*leaf)->PROCESS_PID = 0; // The frame is empty
        (*leaf)->next = NULL;

        /* initialize the children to null */
        (*leaf)->left = NULL;
        (*leaf)->right = NULL;
    }

    else if(FRAME_ID < (*leaf)->FRAME_ID){
        caller->FREE_SPACE = SIZE / 2;
        FRAME_ID += create_frame(caller, &(*leaf)->left);
    }

    else if(FRAME_ID > (*leaf)->FRAME_ID){
        caller->FREE_SPACE = SIZE / 2;
        FRAME_ID += create_frame(caller, &(*leaf)->right);
    }

    return FRAME_ID;
}

void initialize_memory(){
    mainMemory = malloc(sizeof(MEMORY));
    mainMemory->SIZE = MAX_VIRTUAL_PAGES * PAGE_SIZE; // Main memory size in KB. Default is 64 * 4 = 256 KB
    mainMemory->FRAME_ROOT = NULL;
    mainMemory->FREE_SPACE = MAX_VIRTUAL_PAGES * PAGE_SIZE; // The memory is empty
    mainMemory->NFRAMES = 0;
    mainMemory->PREVIOUS_FRAME = 0; // No previous frame

    mainMemory->NFRAMES = create_frame(mainMemory, &mainMemory->FRAME_ROOT); // Creates the first frame occupying all the memory
}

void *Create_Process(void *arg){
    //int idThread = *(int *) arg;
    //clock_t thread_time, temp_time = 0;
    //double aux;

    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (i == 0){ // Don't wait to create the first processes
            printf("*------------------------------First process created------------------------------*\n");

            process_list[i] = Assemble_PCB(pid_counter);
            //process_created++;

            printf("--> process_list[%d] = {pid: %d, virtual pages: %d, size: %d KB}\n", i, process_list[i]->PID, process_list[i]->VIRTUAL_PAGES, process_list[i]->SIZE);
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
            sleep(PROCESS_TIME); //Creates a process each 3 secs

            process_list[i] = Assemble_PCB(pid_counter);
            //process_created++;

            printf("--> process_list[%d] = {pid: %d, virtual pages: %d, size: %d KB}\n", i, process_list[i]->PID, process_list[i]->VIRTUAL_PAGES, process_list[i]->SIZE);
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

void *Memory_Manager(void *arg){



}

void iniciaSwap() {
    swapMemory = malloc(sizeof(SWAP_MEMORY));
    swapMemory->num_pag = 0;
    swapMemory->pag_atual = NULL;
    swapMemory->pag_ultima = swapMemory->pag_atual;
}

int swap_in(int proc_pid){
    int paginas_swap = 0;
    FRAME *temp;

    PAGE *pag_agora = swapMemory->pag_atual;

    while(pag_agora !=NULL){
        if(pag_agora->OWNER_PID == proc_pid){
            paginas_swap++;
        }
        pag_agora = pag_agora->next;
    }

    if(mainMemory->NFRAMES + paginas_swap > MAX_FRAMES){
        printf("No proc_pid %d nao pode realizar swapMemory in. Começando swapMemory out....\n", proc_pid);
        //swap_out(proc_pid);
    }

    printf("Começando swapMemory in do processo : %d\n", proc_pid);


    if(swapMemory->pag_atual != NULL){
        if(swapMemory->pag_atual->OWNER_PID == proc_pid){
            while(swapMemory->pag_atual != NULL && swapMemory->pag_atual->OWNER_PID == proc_pid){
                temp = search_memory(PAGE_SIZE, mainMemory->FRAME_ROOT); // tirar FRAME_ROOT abaixo, colocar search_memory com os devidos parametros
                temp->PAGE_ID = swapMemory->pag_atual->num;
                temp->PROCESS_PID = swapMemory->pag_atual->OWNER_PID;
                //mainMemory->FRAME_ROOT->left->PAGE_ID = swapMemory->pag_atual->num;
                //mainMemory->FRAME_ROOT->left->PROCESS_PID = swapMemory->pag_atual->OWNER_PID;
                swapMemory->pag_atual = swapMemory->pag_atual->next;
                //mainMemory->pag_ultima = mainMemory->FRAME_ROOT->left;
                //mainMemory->FRAME_ROOT->left = NULL;
            }
        }

        else {
            pag_agora = swapMemory->pag_atual;

            while(pag_agora != NULL){
                if(pag_agora->OWNER_PID == proc_pid){
                    mainMemory->FRAME_ROOT->left->PROCESS_PID = pag_agora->OWNER_PID; // colocar temp aqui tambem(?)
                    mainMemory->FRAME_ROOT->left->PAGE_ID = pag_agora->num;
                    pag_agora->next = pag_agora->next->next;
                    mainMemory->FRAME_ROOT->right = mainMemory->FRAME_ROOT->left;
                    mainMemory->FRAME_ROOT->left = NULL;
                }
                pag_agora = pag_agora->next;
            }
        }
        mainMemory->NFRAMES += paginas_swap;
        swapMemory->num_pag -= paginas_swap;
    }

    printf("Acabou o swap in!\n");
    //exibe_memoria();
    //exibe_swap();
    printf("\n");

    return paginas_swap;

}

void swap_out(){

}

void allocate_page(PAGE *page_ins, FRAME *frame_atual){

    if ((search_memory(mainMemory->NFRAMES, frame_atual)) == 0){ // Talvez seja melhor criar um variação de search_memory

        frame_atual->PAGE_ID = page_ins->num;
        frame_atual->PROCESS_PID = page_ins->OWNER_PID;
        frame_atual->FRAME_ID = mainMemory ->NFRAMES;

        if(mainMemory->PREVIOUS_FRAME != 0){
            search_frame(mainMemory->PREVIOUS_FRAME, mainMemory->FRAME_ROOT)->next = frame_atual;
        }

        swap_in(page_ins->OWNER_PID);
    }
    else{
        printf("Exists!");

    }

}

void terminate(){
    for (int i = 0; i < MAX_PROCESSES; i++){
        PAGE *tmp = process_list[i]->first;

        while(tmp != NULL){
            free(tmp);
            tmp = tmp->next;
        }

        free(process_list[i]);
    }
}

void destroy_tree(FRAME *leaf){
    if(leaf != NULL){
        destroy_tree(leaf->left);
        destroy_tree(leaf->right);
        free(leaf);
    }
}

int main(int argc, char const *argv[]){
    //int P[20],burst_time[20],quantum,n; // Round Robin waiting time/ turnaround time variables/ parameters
    //start_t = clock();

    printf("--Starting simulator...\n");
    //printf("==> Simulator start time: %6.3f\n", (start_t * 1000. / CLOCKS_PER_SEC));
    //printf("\n");

    if (sodium_init() < 0) {
        fprintf(stderr, "Panic! The Sodium library couldn't be initialized, it is not safe to use\n");
        return 1;
    }

    printf("Default values: Max frames in main memory = %d, page size = %d KB, max number of pages per process = %d, process working set limit = %d pages\n", MAX_FRAMES, PAGE_SIZE, MAX_VIRTUAL_PAGES, WORKING_SET_LIMIT);
    printf("\n");

    printf("--Initializing memory...\n");

    initialize_memory();

    printf("Main memory initialized. Size = %d KB, current number of frames = %d, free space = %d KB, frame size = %d KB, frame id = %d, id of page in frame = %d, pid of page owner = %d\n", mainMemory->SIZE, mainMemory->NFRAMES, mainMemory->FREE_SPACE, mainMemory->FRAME_ROOT->SIZE, mainMemory->FRAME_ROOT->FRAME_ID, mainMemory->FRAME_ROOT->PAGE_ID, mainMemory->FRAME_ROOT->PROCESS_PID);
    printf("\n");

    //iniciaSwap();

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
            fprintf(stderr, "--ERROR: malloc()\n");
            exit(-1);
        }

        *arg = t;

        if (t == 0) {
            if (pthread_create(&tid_sistema[t], NULL, Create_Process, (void *) arg)) {
                fprintf(stderr, "--ERROR: pthread_create()\n");
                exit(-1);
            }
        }

        if (t == 1){
            if (pthread_create(&tid_sistema[t], NULL, Memory_Manager, (void *) arg)) {
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
            fprintf(stderr, "--ERROR: pthread_join() \n");
            exit(-1);
        }
    }

    terminate();
    destroy_tree(mainMemory->FRAME_ROOT);
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


