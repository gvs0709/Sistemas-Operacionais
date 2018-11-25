#include <unistd.h> // To use fork
#include <wait.h>
#include <string.h> // To use strlen
#include <errno.h>

int prog2v2(){
    //Início
    int child, status = 0, i, argc = 0;
    char com[COMMAND_BUFFER_SIZE], base_path[PATH_SIZE], *token, **argv = malloc(5 * sizeof(char *));

    errno = 0;

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
        i = 0;

        while (token != NULL) {
            argv[i] = token;
            i++;

            token = strtok(NULL, " ");
        }

        argc = i;
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
            strcpy(base_path, "/bin/");
            strcat(base_path, argv[0]);

            switch (argc) {
                case 1:
                    execl(base_path, argv[0], 0);
                    break;

                case 2:
                    execl(base_path, argv[0], argv[1], 0);
                    break;

                case 3:
                    execl(base_path, argv[0], argv[1], argv[2], 0);
                    break;

                case 4:
                    execl(base_path, argv[0], argv[1], argv[2], argv[3], 0);
                    break;

                case 5:
                    execl(base_path, argv[0], argv[1], argv[2], argv[3], argv[4], 0);
                    break;

                default:
                    fprintf(stderr, "-- Foi passada uma quantidade não suportada de argumentos\n");
                    printf("\n");
            }

            exit(errno);
        }

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
                fprintf(stderr, "-- Código de retorno = %d", errno);
                printf("\n");
                errno = 0;
                //Fim
            }
            //Fim se;
        }
    }

    free(argv);
    return 0;
}
