#include <unistd.h> // To use fork
#include <wait.h>
#include <string.h> // To use strlen

int prog2v2(){
    /*Início
        Lê linha de comando;

        Enquanto não fim faça
            Início
                Percorre a linha retirando o nome do comando;
                Executa um fork para criar um novo processo;

                Se processo filho então
                    Executa execl especificando o nome do comando como parâmetro;

                Senão
                    Inicio
                        Executa wait para esperar que a execução do comando termine;

                        Se codigo retorno = zero então
                            Escreva "Executado com sucesso."

                        Senão
                            Escreva "Código de retorno = ", codigo_retorno;
                        Fim
                Fim se;

                Lê linha de comando;
            Fim;
    Fim;
    */

    return 0;
}
