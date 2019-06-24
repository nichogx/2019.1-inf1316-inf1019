#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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
	for (char *p = algoritmo; *p; ++p) *p = tolower(*p);
	char *arquivo = argv[2];
	int tamPag = atoi(argv[3]);
	int tamMem = atoi(argv[4]);

	if (tamPag < 8 || tamPag > 32) {
		erroComando(argv[0], "Tamanho de página deve ser entre 8 e 32 KB.");
	}

	if (tamMem < tamPag) {
		erroComando(argv[0], "Memória deve ser maior que o tamanho das páginas.");
	}

	// abrir arquivo
	FILE *arqFile = fopen(arquivo, "r");
	if (!arqFile) {
		erroComando(argv[0], "Não foi possível abrir o arquivo.");
	}

	puts("Executando o simulador...");
	printf("Arquivo de entrada: %s\n", arquivo);
	printf("Tamanho da memória física: %d KB\n", tamMem);
	printf("Tamanho das páginas: %d KB\n", tamPag);
	printf("Alg de substituição: %s\n", algoritmo);

	// chamar algorítmo correto
	int ret = 0;
	if (strcmp(algoritmo, "lru") == 0) {
		ret = VMEM_inicia(arqFile, tamPag, tamMem, ALG_LRU);
	} else if (strcmp(algoritmo, "nru") == 0) {
		ret = VMEM_inicia(arqFile, tamPag, tamMem, ALG_NRU);
	} else if (strcmp(algoritmo, "novo") == 0) {
		ret = VMEM_inicia(arqFile, tamPag, tamMem, ALG_NOVO);
	} else {
		fclose(arqFile);
		erroComando(argv[0], "Algorítmo não reconhecido.");
	}

	if (ret) {
		puts("ERRO NA SIMULAÇÃO.");
	} else {
		puts("SIMULAÇÃO FINALIZADA.");
	}
	fclose(arqFile);

	return 0;
}
