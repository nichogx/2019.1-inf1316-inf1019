#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "simvirtual.h"

static void erroComando(char *chamado, char *msg) {
	puts("Erro no comando.");
	printf("%s\n", msg);
	printf("Forma de uso: %s <algoritmo LRU|NRU|NOVO> <arquivo> <tamPag> <tamMem>\n", chamado);
	exit(1);
}

int main(int argc, char **argv) {
	if (argc != 5) {
		erroComando(argv[0], "Número de argumentos inválidos.");
	}

	char *algoritmo = argv[1];
	char *arquivo = argv[2];
	int tamPag = atoi(argv[3]);
	int tamMem = atoi(argv[4]);

	// abrir arquivo
	FILE *arqFile = fopen(arquivo, "r");
	if (!arqFile) {
		erroComando(argv[0], "Não foi possível abrir o arquivo.");
	}

	// chamar algorítmo correto
	if (strcmp(algoritmo, "LRU") == 0) {
		VMEM_inicia(arqFile, tamPag, tamMem, ALG_LRU);
	} else if (strcmp(algoritmo, "NRU") == 0) {
		VMEM_inicia(arqFile, tamPag, tamMem, ALG_NRU);
	} else if (strcmp(algoritmo, "NOVO") == 0) {
		VMEM_inicia(arqFile, tamPag, tamMem, ALG_NOVO);
	} else {
		fclose(arqFile);
		erroComando(argv[0], "Algorítmo não reconhecido.");
	}

	fclose(arqFile);

	return 0;
}
