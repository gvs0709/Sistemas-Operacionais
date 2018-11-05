#include <stdio.h>
#include <stdlib.h> // To use scanf
#include <unistd.h> // To use fork

int main(int argc, char const *argv[]){
    int ret1, ret2, example;
    char again;

    while(1){
        printf("Which example to execute? (number from 1 to 4)\n");
        printf(">> ");
        scanf("%d", &example);
        printf("\n");

        switch(abs(example)){
            case 1:
                ret1 = fork(); // Find a way to stop an infinit loop on the child
                ret2 = fork();

                printf("Programa em execução.\n");
                break;

            case 2:
                ret1 = fork();

                if (ret1 == 0){
                    execl("/bin/ls", "ls", 0);
                }

                else{
                    printf("Processo continua executando.\n");
                }

                break;

            case 3:
                ret1 = fork();

                if (ret1 == 0){
                    execl("/bin/ls", "ls", 0);
                    printf("Quando este comando será executado?\n");
                }

                printf("Por que a função printf anterior não foi executada?\n");
                break;

            case 4:
                ret1 = fork();

                if (ret1 == 0){
                    execl("/bin/ll", "ll", 0);
                    printf("Por que este comando foi executado?\n");
                }

                else{
                    printf("Processo continua executando.\n");
                }

                break;

            default:
                printf("-> Input was not a number between 1 and 4\n");
        }

        printf("Execute another example? [Y/n]\n");
        printf(">> ");
        scanf("%s", &again);
        printf("\n");

        if(again == 'n' || again == 'N'){
            break;
        }

    }

    return 0;
}
