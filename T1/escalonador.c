#include <stdio.h>
#include <stdlib.h>

#include <time.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/wait.h>

#include "fila.h"

#define QUANTUM 5

typedef enum {
	CONDRET_RODANDO,
	CONDRET_TERMINOU,
	CONDRET_ESPERANDO_IO,
} tpCondRet;

int terminou = 0;
int waiting = 0;

void sighandler(int signo) {
	if (signo == SIGCHLD) { // atual terminou
		terminou = 1;
	}

	if (signo == SIGUSR1) { // atual está esperando I/O
		waiting = 1;
	}
}

tpCondRet executa(pid_t pid) {
    time_t start;

    // ESPERA O QUANTUM/PROGRAMA TERMINAR
    time(&start);

	terminou = 0;
	signal(SIGCHLD, sighandler);

	// PROGRAMA INICIA/CONTINUA
    kill(pid, SIGCONT);

    while(time(NULL)-start < QUANTUM) {
		if (terminou) { // SIGCHLD modificará terminou para 1
			return CONDRET_TERMINOU;
		}

		if (waiting) { // SIGUSR1 modificará waiting para 1
			// TODO aumentar prioridade
			return CONDRET_ESPERANDO_IO;
		}
	} // se sair do while, não terminou

    kill(pid, SIGSTOP);

	signal(SIGCHLD, SIG_DFL);

    return CONDRET_RODANDO;
}

int main(void) {
    // TODO: receber os programas e seus argumentos (primeiro arg é pid do pai por conta de w4IO)
    //       determinar a prioridade de cada programa (qual fila entra)
    //       executar pela primeira vez, dar stop e colocar na fila um programa
    //       determinar o quantum de cada fila
    //       verificar se o programa é cpu ou I/O bound


	FilaPid *prior[] = {
		FPID_create(),
		FPID_create(),
		FPID_create()
	};

    signal(SIGUSR1,NULL);

	// Criar PIDS de teste

	for (;;) {
		for (int i = 0; i < 3; i++) { // anda pelas filas, prioridade menor para maior
			while (!FPID_isempty(prior[i])) {
				pid_t pid = FPID_dequeue(prior[i]);
				tpCondRet condicao = executa(pid);
				if (condicao == CONDRET_RODANDO) { // foi interrompido? coloca de volta na fila e diminui prioridade
					int novafila = (i + 1 < 2) ? (i + 1) : 2;
					FPID_enqueue(pid, prior[novafila]);
				} else if (condicao == CONDRET_ESPERANDO_IO) { // aumentar prioridade se estiver io bound
					int novafila = i ? (i - 1) : 0;

					// TODO não colocar de volta agora, só depois de 3 segundos (operação IO completa)
					FPID_enqueue(pid, prior[novafila]);
				}
			}
		}

		puts("Terminou todas as filas. Dormindo por 10 segundos...");
		sleep(10);
	}

    return 0;

}
