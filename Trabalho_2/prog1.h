#include <unistd.h> // To use fork and pipe
#include <wait.h>
#include <string.h> // To use strlen

int prog1(){
    int status, id, j, fd[2], aux;
    ssize_t nbytes;
    char *string, readbuffer[80];

    // Insira um comando para pegar o PID do processo corrente e mostre na tela da console.
    id = getpid();

    printf("PID do processo corrente (pai): %d\n", id);
    pipe(fd);

    if ((aux = fork()) > 0/* insira um comando para criar um subprocesso*/){ // Faça com que o processo pai execute este trecho de código
        //Mostre na console o PID do processo pai e do processo filho
        printf("[PAI] PID do pai: %d, PID do filho: %d\n", id, aux);

        //Monte uma mensagem e a envie para o processo filho
        string = "Eu sou seu pai!";

        close(fd[0]);  // Parent process closes up input side of pipe
        write(fd[1], string, (strlen(string)+1)); // Send "string" through the output side of pipe

        //Mostre na tela o texto da mensagem enviada
        printf("[PAI] Menssagem enviada: '%s'\n", string);

        //Aguarde a resposta do processo filho
        wait(&status);
        close(fd[1]); // Parent process closes up output side of pipe

        nbytes = read(fd[0], readbuffer, sizeof(readbuffer)); //Read "readbuffer" through the input side of pipe

        //Mostre na tela o texto recebido do processo filho
        printf("[PAI] Menssagem recebida: '%s'\n", readbuffer);

        //Aguarde mensagem do filho e mostre o texto recebido
        wait(&status);
        close(fd[1]); // Parent process closes up output side of pipe

        nbytes = read(fd[0], readbuffer, sizeof(readbuffer)); //Read "readbuffer" through the input side of pipe

        printf("[PAI] Menssagem recebida: '%s'\n", readbuffer);

        //Aguarde o término do processo filho
        wait(&status);

        //Informe na tela que o filho terminou e que o processo pai também vai encerrar
        printf("[PAI] Processo filho terminou, encerrando...\n");
    }

    else { //Faça com que o processo filho execute este trecho de código
        //Mostre na tela o PID do processo corrente e do processo pai
        printf("[FILHO] PID do processo corrente: %d, PID do pai: %d\n", getpid(), id);

        //Aguarde a mensagem do processo pai e ao receber mostre o texto na tela
        close(fd[1]); // Child process closes up output side of pipe
        nbytes = read(fd[0], readbuffer, sizeof(readbuffer)); //Read "readbuffer" through the input side of pipe

        printf("[FILHO] Menssagem recebida: '%s'\n", readbuffer);

        //Envie uma mensagem resposta ao pai
        string = "Nããããããooooo!!!";

        close(fd[0]);  // Child process closes up input side of pipe
        write(fd[1], string, (strlen(string)+1)); // Send "string" through the output side of pipe

        //Execute o comando “for” abaixo
        /*for (j = 0; j <= 10000; j++){
        // Envie mensagem ao processo pai com o valor final de “j”
        //Execute o comando abaixo e responda às perguntas

            execl("/bin/ls", "ls", NULL);

        // O que acontece após este comando?
        //O que pode acontecer se o comando “execl” falhar?
        }*/

        exit(0);
    }

    //exit(0);
    return 0;
}
