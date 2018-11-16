#include <unistd.h> // To use fork
#include <wait.h>

int prog1(){
    int status, id, j;

    // Insira um comando para pegar o PID do processo corrente e mostre na tela da console.
    id = getpid();

    printf("PID do processo corrente (pai): %d", id);

    if ((id = fork()) != 0/* insira um comando para criar um subprocesso*/){ // Faça com que o processo pai execute este trecho de código
        //Mostre na console o PID do processo pai e do processo filho
        printf("PID do pai: %d, PID do filho: %d", getpid(), id);
        //Monte uma mensagem e a envie para o processo filho
        //Mostre na tela o texto da mensagem enviada
        //Aguarde a resposta do processo filho
        //Mostre na tela o texto recebido do processo filho
        //Aguarde mensagem do filho e mostre o texto recebido
        //Aguarde o término do processo filho
        //Informe na tela que o filho terminou e que o processo pai também vai encerrar
    }

    else { //Faça com que o processo filho execute este trecho de código
        //Mostre na tela o PID do processo corrente e do processo pai
        printf("PID do processo corrente (filho): %d, PID do pai: %d", id, getppid());
        //Aguarde a mensagem do processo pai e ao receber mostre o texto na tela
        //Envie uma mensagem resposta ao pai
        //Execute o comando “for” abaixo

        for (j = 0; j <= 10000; j++){
        /* Envie mensagem ao processo pai com o valor final de “j”
        Execute o comando abaixo e responda às perguntas*/

            execl("/bin/ls", "ls", NULL);

        /* O que acontece após este comando?
        O que pode acontecer se o comando “execl” falhar?*/
        }
    }

    exit(0);

    return 0;
}
