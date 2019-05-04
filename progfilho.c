#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main (int argc, char **argv) {
	for (int arg = 1; arg < argc; arg++) {
		for (int i = 0; i < atoi(argv[arg]); i++) {
			printf("%d\n", getpid());
			sleep(1);
		}
	}
}
