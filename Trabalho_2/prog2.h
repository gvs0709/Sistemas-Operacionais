#include <unistd.h> // To use fork
#include <wait.h>
#include <string.h> // To use strlen
#include <errno.h>

enum{
    COMMAND_SIZE = 100, TK_SIZE = 50, PATH_SIZE = TK_SIZE + 10
};

int prog2(){
    //Início
    int child, status, first = 1;
    char com[COMMAND_SIZE], base_path[PATH_SIZE], *token, tk[TK_SIZE];

    //Lê linha de comando;
    printf("$: ");

    do{
        fgets(com, sizeof(com), stdin);
    } while(com[0] == '\n');

    token = strtok(com, " ");

    //Enquanto não fim faça
    while(token != NULL){
        //Início
        //Percorre a linha retirando o nome do comando;
        strcpy(base_path, "/bin/");

        if(first){
            strcpy(tk, token);
            first = 0;
        }

        else{
            strncpy(tk, token, strlen(token)-1);
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
                printf("-- Executado com sucesso.");
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
        token = strtok(NULL, " ");

        /*if(token[0] != '&'){
            printf("-- Uso errado da linha de comando" );
            break;
        }

        if(token[0] == '&'){
            token = strtok(NULL, " ");
        }*/
        //Fim;
    }
    //Fim;
    
    return 0;
}
