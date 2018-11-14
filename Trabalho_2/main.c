#include <stdio.h>
#include <stdlib.h> // To use scanf
#include "exemplos_de_uso.h"
#include "prog1.h"
#include "prog2.h"

int main(int argc, char const *argv[]){
    int func, myPID = getpid();
    char again;

    printf("********************************************************\n");
    printf("\n");
    printf("            Trabalho 2 - S.O. - 2018-2 UFRJ             \n");
    printf("\n");
    printf("********************************************************\n");
    printf("\n");

    while(1){
        printf("Which function to execute (1~3)? [1. ex; 2. p1; 3. p2] >> ");
        scanf("%d", &func);
        printf("\n");

        switch(func){
            case 1:
                exemplos();
                break;

            case 2:
                prog1();
                break;

            case 3:
                prog2();
                break;

            default:
                printf("Input was not one of the options");
                printf("\n");
        }

        if(myPID == getpid()) {
            printf("Execute another function? [Y/n] >> ");
            scanf("%s", &again);
            printf("\n");

            if (again == 'n' || again == 'N') {
                break;
            }
        }

        else{
            break;
        }
    }

    return 0;
}

