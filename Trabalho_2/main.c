#include <stdio.h>
#include <stdlib.h> // To use scanf
#include "exemplos_de_uso.h"
#include "prog1.h"
#include "prog2.h"
#include "prog2v2.h"

int main(int argc, char const *argv[]){
    char func, again;
    int c;

    printf("********************************************************\n");
    printf("\n");
    printf("            Trabalho 2 - S.O. - 2018-2 UFRJ             \n");
    printf("\n");
    printf("********************************************************\n");
    printf("\n");

    while(1){
        printf("Qual função executar (0~3)? [0. exemplos; 1. prog1; 2. prog2; 3. prog2 args] >> ");
        scanf("%c", &func);
        printf("\n");

        while((c = getchar()) != EOF) {
            if (c == '\n') {
                break;
            }
        }

        switch(func){
            case '0':
                exemplos();
                printf("\n");
                break;

            case '1':
                prog1();
                printf("\n");
                break;

            case '2':
                prog2();
                printf("\n");
                break;

            case '3':
                prog2v2();
                printf("\n");
                break;

            default:
                printf("Entrada não era uma das opções\n");
                printf("\n");
        }

        printf("Executar outra função? [S/n] >> ");
        scanf("%c", &again);
        printf("\n");

        while((c = getchar()) != EOF) {
            if (c == '\n') {
                break;
            }
        }

        if (again == 'n' || again == 'N') {
            break;
        }
    }

    return 0;
}

