#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <time.h>
#include <signal.h>

#include <pthread.h>

#include <sys/types.h>
#include <sys/wait.h>

#include "fila.h"
#include "semaforo.h"

#define true 1
#define false 0

// estado de um processo
typedef enum {
	CONDRET_RODANDO,
	CONDRET_TERMINOU,
	CONDRET_ESPERANDO_IO,
} tpCondRet;

typedef struct paramThread {
	FilaPid *fila;
	pid_t pid;
} ParamThread;

// controles de término e IO
static int terminou = false;
static int waiting = false;

// semáforos
static int semId;

// filas
static FilaPid *prior[3];

// função que cuida dos sinais SIGCHLD e SIGUSR1. Modifica as variáveis terminou ou waiting
// para o escalonador pegar.
static void sighandler(int signo) {
	if (signo == SIGCHLD) { // atual terminou
		terminou = true;
	}

	if (signo == SIGUSR1) { // atual está esperando I/O
		waiting = true;
	}
}

// função que finaliza o programa (semáforos, filas, memória compartilhada, etc...)
// pode ser chamada por SIGINT
static void finalize(int signo) {
	delSemValue(semId);

	FPID_destroy(prior[0]);
	FPID_destroy(prior[1]);
	FPID_destroy(prior[2]);

	if (signo != 0) { // foi chamado por sinal e não pela main
		exit(0);
	}
}

// executa um processo pelo seu pid e por um determinado tempo(quantum)
// retorna o estado do processo ao fim da execução
static tpCondRet executa(pid_t pid, int quantum) {
    time_t start;

    // ESPERA O QUANTUM/PROGRAMA TERMINAR
    time(&start);

	terminou = false;
	waiting = false;
	signal(SIGCHLD, sighandler);
	signal(SIGUSR1, sighandler);

	printf("FILHO %d SENDO ESCALONADO\n", pid);
	// PROGRAMA INICIA/CONTINUA
    kill(pid, SIGCONT);

    while(time(NULL)-start < quantum && !waiting) { // SIGUSR1 modificará waiting para true
		if (terminou) { // SIGCHLD modificará terminou para true
			int status;
			waitpid(pid, &status, WNOHANG);
			if (WIFSIGNALED(status)) { // não terminou, foi parado pelo escalonador
				terminou = false;
			} else { // terminou
				puts("FILHO TERMINOU");
				return CONDRET_TERMINOU;
			}
		}
	} // se sair do while, não terminou

    kill(pid, SIGSTOP);

	signal(SIGCHLD, SIG_DFL);
	signal(SIGUSR1, SIG_DFL);

	if (waiting) {
		puts("FILHO ESPERANDO IO");
		return CONDRET_ESPERANDO_IO;
	}

	puts("FILHO ESGOTOU QUANTUM");
    return CONDRET_RODANDO;

}

// função 'main' da thread que irá colocar de volta o PID na fila após o fim do IO
static void *threadEnqueue(void *params) {
	ParamThread *p = (ParamThread *) params;

	sleep(3);
	puts("COLOCANDO DNV");
	semaforoP(semId);
		FPID_enqueue(p->pid, p->fila);
	semaforoV(semId);

	free(p);

	pthread_exit(NULL);
}

// função para calcular o valor da potência de a elevado a b
static int expo(int a, int b) {
	if (b == 0) {
		return 1;
	}

	return a * expo(a, b - 1);
}

int main(void) {
    // TODO: receber os programas e seus argumentos

	puts("TRABALHO 1 DE SC/SO - ESCALONADOR");
	puts("Por Alexandre Heine e Nicholas Godoy");
	puts("GitHub: https://github.com/nichogx/2019.1-inf1316-inf1019/tree/master/T1");
	puts("\nPara sair digite ^C\n");

	// setar semáforos
	semId = semget (IPC_PRIVATE, 1, 0666 | IPC_CREAT);
	setSemValue(semId);
	signal(SIGINT, finalize);

	// criar as filas
	prior[0] = FPID_create();
	prior[1] = FPID_create();
	prior[2] = FPID_create();

	// programas de teste
	pid_t pidpai = getpid();
	pid_t filho = 0;
	if ((filho = fork()) == 0) {
		char argpidpai[20];
		sprintf(argpidpai, "%d", pidpai);
		execl("./progfilho", "./progfilho", argpidpai, "3", "3", "3", NULL);
	} else {
		kill(filho, SIGSTOP);
		puts("PAROU FILHO NO CREATE");

		semaforoP(semId);
			FPID_enqueue(filho, prior[0]);
		semaforoV(semId);
	}

	sleep(2);

	int cont = 0;
	while(cont < 10) { // parar se passar 10 segundos sem atividade
		for (int i = 0; i < 3; i++) { // anda pelas filas, prioridade menor para maior
			int quantum = expo(2, i);
			int qtd = 4 / quantum; // 4 segundos para cada fila
			for (int j = 0; j < qtd && !FPID_isempty(prior[i]); j++) {
				cont = 0; // reseta para o while não parar

				semaforoP(semId);
					pid_t pid = FPID_dequeue(prior[i]);
				semaforoV(semId);

				printf("FILHO %d NO NIVEL %d VAI SER ESCALONADO\n", pid, i);
				tpCondRet condicao = executa(pid, quantum);
				if (condicao == CONDRET_RODANDO) { // foi interrompido? coloca de volta na fila e diminui prioridade
					int novafila = (i + 1 < 2) ? (i + 1) : 2;

					semaforoP(semId);
						FPID_enqueue(pid, prior[novafila]);
					semaforoV(semId);
				} else if (condicao == CONDRET_ESPERANDO_IO) { // aumentar prioridade se estiver io bound
					int novafila = i ? (i - 1) : 0;

					ParamThread *params = (ParamThread *) malloc (sizeof(ParamThread));
					if (!params) {
						puts("ERRO DE MEMÓRIA (malloc)");
						exit(1);
					}

					params->fila = prior[novafila];
					params->pid = pid;

					pthread_t thread;
					pthread_create(&thread, NULL, threadEnqueue, (void *) params);
				}
			}
		}

		puts("Terminou todas as filas. Dormindo por 1 segundo...");
		cont++;
		sleep(1);
	}

	puts("10 SEGUNDOS SEM ATIVIDADE. TERMINANDO PROGRAMA.");

	finalize(0);
    return 0;
}
