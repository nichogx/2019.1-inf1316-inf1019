#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
	CONDRET_MANTER_FILA,
} tpCondRet;

typedef struct paramThread {
	FilaPid *fila;
	pid_t pid;
} ParamThread;

// controles de término e IO
static int terminou = false;
static int waiting = false;
static int esccount = 0;

// semáforos
static int semId;

// filas
static FilaPid *prior[3];
static FilaPid *temp;

// função que cuida dos sinais SIGCHLD e SIGUSR1. Modifica as variáveis terminou ou waiting
// para o escalonador pegar.
static void sighandler(int signo) {
	if (signo == SIGCHLD) { // atual terminou
		terminou = true;
	}

	if (signo == SIGUSR1) { // atual está esperando I/O
		waiting = true;
	}

	if (signo == SIGUSR2) { // atual terminou 1s do filho
		esccount++;
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
		FPID_destroy(temp);
	}
}

// executa um processo pelo seu pid e por um determinado tempo(quantum)
// retorna o estado do processo ao fim da execução
static tpCondRet executa(pid_t pid, int quantum) {
	terminou = false;
	waiting = false;
	signal(SIGCHLD, sighandler);
	signal(SIGUSR1, sighandler);
	signal(SIGUSR2, sighandler);

	printf("FILHO %d SENDO ESCALONADO\n", pid);

	esccount = 0; // SIGUSR2 incrementa
    while (esccount < quantum && !waiting) { // SIGUSR1 modificará waiting para true
		kill(pid, SIGCONT);
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
	signal(SIGUSR2, SIG_DFL);

	if (waiting) {
		if (esccount == quantum) {
			puts("FILHO ESPERANDO IO (TEMPO BATEU COM QUANTUM)");
			return CONDRET_MANTER_FILA;
		}

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
	printf("FILHO %d TERMINOU I/O, COLOCANDO NA FILA\n", p->pid);
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
	puts("\nPara sair digite ^C ou espere 10 segundos após todos os processos terminarem.\n");

	puts("Digite comandos... Lista de comandos:\n");
	puts("    exec ./progfilho t1 t2 t3...  - para colocar um programa na fila");
	puts("    start                         - para começar o escalonador\n");

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

	char spidpai[20];
	sprintf(spidpai, "%d", pidpai);

	char inBuf[201];
	char cmd[11];
	printf(" > ");
	scanf("%10s", cmd);
	while (strcmp(cmd, "exec") == 0) {
		scanf(" %200[^\n]", inBuf);

		char *argv[12];
		int argc = 0;
		argv[1] = spidpai;

		char *str = strtok (inBuf, " ");
		while (str != NULL) {
			if (argc == 1) argc++;
			argv[argc++] = strdup(str);
			str = strtok (NULL, " ");
		}
		argv[argc] = NULL;

		if ((filho = fork()) == 0) {
			execv(argv[0], argv);
		} else {
			kill(filho, SIGSTOP);
			puts("Programa foi colocado na fila.");

			semaforoP(semId);
				FPID_enqueue(filho, prior[0]);
			semaforoV(semId);
		}

		printf(" > ");
		scanf("%10s", cmd);
	}

	sleep(2);

	int cont = 0;
	while(cont < 10) { // parar se passar 10 segundos sem atividade
		for (int i = 0; i < 3; i++) { // anda pelas filas, prioridade menor para maior
			int quantum = expo(2, i);
			int qtd = expo(2, 2 - i);
			for (int j = 0; j < qtd && !FPID_isempty(prior[i]); j++) {
				cont = 0; // reseta para o while não parar

				temp = FPID_create(); // fila pra reinserir

				while (!FPID_isempty(prior[i])) {
					semaforoP(semId);
						pid_t pid = FPID_dequeue(prior[i]);
					semaforoV(semId);

					printf("FILHO %d NO NIVEL %d VAI SER ESCALONADO\n", pid, i);

					tpCondRet condicao = executa(pid, quantum);
					if (condicao == CONDRET_RODANDO) { // foi interrompido? coloca de volta na fila e diminui prioridade
						int novafila = (i + 1 < 2) ? (i + 1) : 2;

						if (novafila == i) {
							semaforoP(semId);
								FPID_enqueue(pid, temp);
							semaforoV(semId);
						} else {
							semaforoP(semId);
								FPID_enqueue(pid, prior[novafila]);
							semaforoV(semId);
						}
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
					} else if (condicao == CONDRET_MANTER_FILA) { // manter na mesma fila
						ParamThread *params = (ParamThread *) malloc (sizeof(ParamThread));
						if (!params) {
							puts("ERRO DE MEMÓRIA (malloc)");
							exit(1);
						}

						params->fila = prior[i];
						params->pid = pid;

						pthread_t thread;
						pthread_create(&thread, NULL, threadEnqueue, (void *) params);
					}
				}

				while (!FPID_isempty(temp)) {
					FPID_enqueue(FPID_dequeue(temp), prior[i]);
				}

				FPID_destroy(temp);
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
