#include <unistd.h> // To use fork
#include <wait.h>
#include <string.h> // To use strlen
#include <errno.h>

int prog2v2(){ // Em progresso!!! Ainda igual a prog2
    //Início
    int child, status, sys_call;
    char com[COMMAND_BUFFER_SIZE], base_path[PATH_SIZE], *token, tk[TK_SIZE], find_command[6];

    while(1) {
        //Lê linha de comando;
        printf("[terminal]$: ");

        do {
            fgets(com, COMMAND_BUFFER_SIZE, stdin);
        } while (com[0] == '\n');

        if ((strlen(com) > 0) && (com[strlen(com) - 1] == '\n')) { // Remove trailing newline, if there.
            com[strlen(com) - 1] = '\0';
        }

        if(com[0] == 'e' && com[1] == 'x' && com[2] == 'i' && com[3] == 't' && com[4] == '\0'){
            printf("-- Saindo do terminal...\n");
            break;
        }

        token = strtok(com, " ");

        //Enquanto não fim faça
        while (token != NULL) {
            //Início
            //Percorre a linha retirando o nome do comando;
            strcpy(base_path, "/bin/");
            strcpy(tk, token);

            token = strtok(NULL, " "); //Lê linha de comando

            if (token != NULL) {
                if (token[0] != '&') {
                    fprintf(stderr, "-- Uso comando & comando");
                    printf("\n");
                    break;
                }

                if (token[0] == '&') { // Ignora o &
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

            //Lê linha de comando
            /*token = strtok(NULL, " ");

            if (token != NULL) {
                if (token[0] != '&') {
                    printf("-- Para executar 2 comandos seguidos é necessário '&'");
                    break;
                }

                if(token[0] == '&'){ // Ignora o &
                    token = strtok(NULL, " ");
                }
            }*/
            //Fim;
        }
        //Fim;
    }

    return 0;
}
