#include <unistd.h> // To use fork
#include <wait.h>
#include <string.h> // To use strlen
#include <errno.h>

enum{
    COMMAND_BUFFER_SIZE = 100, TK_SIZE = 50, PATH_SIZE = TK_SIZE + 10
};

int prog2(){
    //Início
    int child, status = 0, sys_call, leave = 0;
    char com[COMMAND_BUFFER_SIZE], base_path[PATH_SIZE], *token, tk[TK_SIZE], find_command[6];

    errno = 0;

    while(1) {
        //Lê linha de comando;
        printf("[terminal]$: ");

        do {
            fgets(com, COMMAND_BUFFER_SIZE, stdin);
        } while (com[0] == '\n');

        if ((strlen(com) > 0) && (com[strlen(com) - 1] == '\n')) { // Remove trailing newline, if there.
            com[strlen(com) - 1] = '\0';
        }

        /*if(com[0] == 'e' && com[1] == 'x' && com[2] == 'i' && com[3] == 't' && com[4] == '\0'){
            printf("-- Saindo do terminal...\n");
            break;
        }*/

        token = strtok(com, " ");

        //Enquanto não fim faça
        while (token != NULL) {
            //Início
            if (strcmp(token, "exit") == 0){
                printf("-- Saindo do terminal...\n");
                leave = 1;
                break;
            }

            //Percorre a linha retirando o nome do comando;
            strcpy(base_path, "/bin/");
            strcpy(tk, token);

            token = strtok(NULL, " "); //Lê linha de comando

            if (token != NULL) {
                if (token[0] != '&') {
                    fprintf(stderr, "-- Uso: comando & comando");
                    printf("\n");
                    break;
                }

                if (token[0] == '&') { // Ignora o '&'
                    token = strtok(NULL, " ");
                }
            }

            strcat(base_path, tk);

            strcpy(find_command, "find ");
            strcat(find_command, base_path);
            strcat(find_command, " >/dev/null 2>&1");
            sys_call = system(find_command);

            if (sys_call > 0) {
                fprintf(stderr, "\n-- Comando inexistente\n");
                continue;
            }

            //Executa um fork para criar um novo processo;
            child = fork();

            if (child < 0) {
                fprintf(stderr, "-- Fork Failed\n");
                return 1;
            }

            //Se processo filho então
            if (child == 0) {
                //Executa execl especificando o nome do comando como parâmetro;
                execl(base_path, tk, (char *) 0);
                exit(errno);
            }

            //Senão
            else {
                //Inicio
                //Executa wait para esperar que a execução do comando termine;
                //wait(&status);
                waitpid(child, &status, WUNTRACED);
                status = 0;

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
                    fprintf(stderr, "-- Código de retorno = %d\n", errno);
                    printf("\n");
                    errno = 0;
                    //Fim
                }
                //Fim se;
            }
            //Fim;
        }
        //Fim;

        if(leave){
            break;
        }
    }
    
    return 0;
}
