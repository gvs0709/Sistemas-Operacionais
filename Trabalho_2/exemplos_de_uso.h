#include <unistd.h> // To use fork

int exemplos(){
    int ret1 = 1 + getpid(), ret2 = 1, c;
    char example, again;

    while(1){
        printf("Qual exemplo executar? (número de 1 a 4) >> ");
        scanf("%c", &example);
        printf("\n");

        while((c = getchar()) != EOF) {
            if (c == '\n') {
                break;
            }
        }

        switch(example){
            case '1':
                ret1 = fork();
                ret2 = fork();

                printf("\n");
                printf("--Programa em execução.\n");
                break;

            case '2':
                ret1 = fork();

                if (ret1 == 0){
                    printf("\n");
                    execl("/bin/ls", "ls", (char *)0);
                }

                else{
                    printf("\n");
                    printf("--Processo continua executando.\n");
                }

                break;

            case '3':
                ret1 = fork();

                if (ret1 == 0){
                    printf("\n");
                    execl("/bin/ls", "ls", (char *)0);
                    //execl("/bin/ps", "ps -ef", (char *)0);
                    printf("--Quando este comando será executado?\n");
                }

                printf("\n");
                printf("--Por que a função printf anterior não foi executada?\n");
                break;

            case '4':
                ret1 = fork();

                if (ret1 == 0){
                    printf("\n");
                    execl("/bin/ll", "ll", (char *)0);
                    //execl("/bin/ls", "ls -lh", (char *)0);
                    printf("--Por que este comando foi executado?\n");
                }

                else{
                    printf("\n");
                    printf("--Processo continua executando.\n");
                }

                break;

            default:
                printf("\n");
                printf("--> Entrada não era um número de 1 a 4\n");
        }

        if(ret2 > 0 && (ret1 - getpid()) > 0) {
            printf("Executar outro exemplo? [S/n] >> ");
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

        else{
            exit(0);
        }

    }

    return 0;
}
