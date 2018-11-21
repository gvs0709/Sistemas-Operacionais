#include <unistd.h> // To use fork and pipe
#include <wait.h>
#include <string.h> // To use strlen

int prog1(){
    int status, id, j, pf[2], fp[2], aux; // pf é um pipe onde o pai escreve e o filho le, fp é um pipe onde o filho escreve e o pai le
    //ssize_t nbytes;
    char *string, readbuffer[80];
    char js[30];

    // Insira um comando para pegar o PID do processo corrente e mostre na tela da console.
    id = getpid();

    printf("PID do processo corrente (pai): %d\n", id);

    if(pipe(pf) == -1){
        fprintf(stderr, "Pipe Failed" );
        return 1;
    }

    if(pipe(fp) == -1){
        fprintf(stderr, "Pipe Failed" );
        return 1;
    }

    aux = fork(); /* insira um comando para criar um subprocesso*/

    if(aux < 0){
        fprintf(stderr, "fork Failed" );
        return 1;
    }

    if(aux > 0){ // Faça com que o processo pai execute este trecho de código
        //Mostre na console o PID do processo pai e do processo filho
        printf("[PAI] PID do pai: %d, PID do filho: %d\n", id, aux);

        //Monte uma mensagem e a envie para o processo filho
        string = "Eu sou seu pai!";

        close(pf[0]);  // Parent process closes up input side of pipe pf
        write(pf[1], string, (strlen(string)+1)); // Send "string" through the output side of pipe pf

        //Mostre na tela o texto da mensagem enviada
        printf("[PAI] Mensagem enviada: '%s'\n", string);
        close(pf[1]);

        //Aguarde a resposta do processo filho
        wait(&status);
        close(fp[1]); // Parent process closes up output side of pipe fp

        read(fp[0], readbuffer, sizeof(readbuffer)); //Read "readbuffer" through the input side of pipe fp

        //Mostre na tela o texto recebido do processo filho
        printf("[PAI] Mensagem recebida 1: '%s'\n", readbuffer);

        //Aguarde mensagem do filho e mostre o texto recebido
        wait(&status);

        read(fp[0], readbuffer, sizeof(readbuffer)); //Read "readbuffer" through the input side of pipe fp

        printf("[PAI] Mensagem recebida 2: '%s'\n", readbuffer);

        //Aguarde o término do processo filho
        printf("[PAI] Esperando processo filho terminar...\n");
        wait(&status);

        //Informe na tela que o filho terminou e que o processo pai também vai encerrar
        printf("[PAI] Processo filho terminou, encerrando...\n");
    }

    else{ //Faça com que o processo filho execute este trecho de código
        //Mostre na tela o PID do processo corrente e do processo pai
        printf("[FILHO] PID do processo corrente: %d, PID do pai: %d\n", getpid(), id);

        //Aguarde a mensagem do processo pai e ao receber mostre o texto na tela
        close(pf[1]); // Child process closes up output side of pipe pf

        read(pf[0], readbuffer, sizeof(readbuffer)); //Read "readbuffer" through the input side of pipe pf

        printf("[FILHO] Mensagem recebida: '%s'\n", readbuffer);
        close(pf[0]);

        //Envie uma mensagem resposta ao pai
        string = "Naaaaaaooooo!!!";

        close(fp[0]);  // Child process closes up input side of pipe fp
        write(fp[1], string, (strlen(string)+1)); // Send "string" through the output side of pipe fp
        //printf("[FILHO] Resposta enviada: '%s'\n", string);

        //Execute o comando “for” abaixo

        for(j = 0; j <= 10000; j++){}
    
        // Envie mensagem ao processo pai com o valor final de “j”
        sprintf(js, "%d", j);
        string = js;

        write(fp[1], string, (strlen(string)+1)); // Send "string" through the output side of pipe fp
        printf("[FILHO] Valor de j: '%s'\n", string);

        //Execute o comando abaixo e responda às perguntas
        //execl("/bin/ls", "ls", NULL);

        // O que acontece após este comando?
        //O que pode acontecer se o comando “execl” falhar?

        exit(0);
    }

    //exit(0);
    return 0;
}
