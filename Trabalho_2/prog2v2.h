#include <unistd.h> // To use fork
#include <wait.h>
#include <string.h> // To use strlen
#include <errno.h>

enum{
    COM_BUFFER_SIZE = 100, TOK_SIZE = 50, PATH_BUFFER_SIZE = TOK_SIZE + 10
};

int prog2v2(){ //Nao terminado
    //Início
    int child, status;
    char com[COM_BUFFER_SIZE], base_path[PATH_BUFFER_SIZE], *token, tk[TOK_SIZE];

    //Lê linha de comando;
    printf("[terminal]$: ");

    do{
        fgets(com, COM_BUFFER_SIZE, stdin);
    } while(com[0] == '\n');

    if ((strlen(com) > 0) && (com[strlen(com) - 1] == '\n')) { // Remove trailing newline, if there.
        com[strlen(com) - 1] = '\0';
    }

    token = strtok(com, " ");

    //Enquanto não fim faça
    while(token != NULL){
        //Início
        //Percorre a linha retirando o nome do comando;
        strcpy(base_path, "/bin/");
        strcpy(tk, token);

        token = strtok(NULL, " "); //Lê linha de comando

        if (token != NULL) {
            if (token[0] != '&') {
                printf("-- Uso comando & comando");
                printf("\n");
                break;
            }

            if(token[0] == '&'){ // Ignora o &
                token = strtok(NULL, " ");
            }
        }

        strcat(base_path, tk);

        //Executa um fork para criar um novo processo;
        child = fork();

        if(child < 0){
            fprintf(stderr, "-- Fork Failed\n");
            return 1;
        }

        //Se processo filho então
        if(child == 0){
            //Executa execl especificando o nome do comando como parâmetro;
            execl(base_path, tk, (char *)0);
            exit(errno);
        }

            //Senão
        else{
            //Inicio
            //Executa wait para esperar que a execução do comando termine;
            wait(&status);

            //Se codigo retorno = zero então
            if(errno == 0){
                //Escreva "Executado com sucesso."
                //printf("\n");
                printf("-- Executado com sucesso ('%s').", tk);
                printf("\n");
            }

                //Senão
            else{
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

    return 0;
}
