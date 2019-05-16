#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>

/*
 * ./progfilho <pid_pai> <segundos_1> <segundos_2> ...
 */
int main (int argc, char **argv) {
	// primeiro argumento precisa ser o PID do pai
	if (argc < 2) {
		puts("Argumentos incorretos\n");
		exit(0);
	}

	pid_t pidpai = atoi(argv[1]);
	printf("pai: %d\n", pidpai);

	for (int arg = 2; arg < argc; arg++) {
		printf("segs: %d\n", atoi(argv[arg]));
		for (int i = 0; i < atoi(argv[arg]); i++) {
			printf("%d\n", getpid());
			sleep(1);
			kill(pidpai, SIGUSR2);

			// se não for a última rajada, parar pro escalonador decidir
			// se continua ou não
			if (i != atoi(argv[arg]) - 1) raise(SIGSTOP);
		}

		kill(pidpai, SIGUSR1); // waiting for I/O
	}
}
