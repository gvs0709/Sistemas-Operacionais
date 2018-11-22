#include <unistd.h> // To use fork
#include <wait.h>
#include <string.h> // To use strlen
#include <errno.h>

int prog2v2(){ // Em progresso!!! Ainda igual a prog2
    //Início
    int child, status, sys_call, i = 0;
    char com[COMMAND_BUFFER_SIZE], base_path[PATH_SIZE], *token, tk[TK_SIZE], find_command[6], **argv = malloc(8 * sizeof(char *));

    while(1) {
        //Lê linha de comando;
        printf("[terminal v2]$: ");

        do {
            fgets(com, COMMAND_BUFFER_SIZE, stdin);
        } while (com[0] == '\n');

        if ((strlen(com) > 0) && (com[strlen(com) - 1] == '\n')) { // Remove trailing newline, if there.
            com[strlen(com) - 1] = '\0';
        }

        token = strtok(com, " ");

        while (token != NULL) {
            argv[i] = token;
            i++;

            token = strtok(NULL, " ");
        }

        argv[i] = NULL;

        if (strcmp(argv[0], "exit") == 0){
            printf("-- Saindo do terminal...\n");
            break;
        }

        child = fork(); //Executa um fork para criar um novo processo;

        if (child < 0) {
            fprintf(stderr, "-- Fork Failed\n");
            return 1;
        }

        if (child == 0) {  //Se processo filho então
            //Executa execl especificando o nome do comando como parâmetro;
            execvp(argv[0], argv);
            exit(errno);
        }

        else {
            //Inicio
            //Executa wait para esperar que a execução do comando termine;
            wait(&status);

            //Se codigo retorno = zero então
            if (errno == 0) {
                //Escreva "Executado com sucesso."
                //printf("\n");
                printf("-- Executado com sucesso.\n");
                printf("\n");
            }

                //Senão
            else {
                //Escreva "Código de retorno = ", codigo_retorno;
                printf("-- Código de retorno = %d", errno);
                printf("\n");
                //Fim
            }
            //Fim se;
        }
    }

    free(argv);
    return 0;
}
