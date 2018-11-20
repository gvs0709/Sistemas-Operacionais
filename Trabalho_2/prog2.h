#include <unistd.h> // To use fork
#include <wait.h>
#include <string.h> // To use strlen
#include <errno.h>

int prog2(){
    //Início
    int child, status;
    char com[100], base_path[60], *token, tk[50];

    //Lê linha de comando;
    printf("$: ");
    scanf("%s", com);
    //printf("\n");

    token = strtok(com, " ");

    //Enquanto não fim faça
    while(token != NULL){
        //Início
        //Percorre a linha retirando o nome do comando;
        strcpy(base_path, "/bin/");
        strcpy(tk, token);
        strcat(base_path, tk);

        //Executa um fork para criar um novo processo;
        child = fork();

        if(child < 0){
            fprintf(stderr, "-- Fork Failed" );
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
                printf("\n");
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
        //Fim;
    }
    //Fim;
    
    return 0;
}
