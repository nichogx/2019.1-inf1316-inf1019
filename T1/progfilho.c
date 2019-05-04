#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>

int main (int argc, char **argv) {
	// primeiro argumento precisa ser o PID do pai
	if (argc < 2) {
		puts("Argumentos incorretos\n");
		exit(0);
	}

	pid_t pidpai = atoi(argv[1]);

	for (int arg = 2; arg < argc; arg++) {
		for (int i = 0; i < atoi(argv[arg]); i++) {
			printf("%d\n", getpid());
			sleep(1);

			kill(pidpai, SIGUSR1); // waiting for I/O
		}
	}
}
