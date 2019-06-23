#include "simvirtual.h"

typedef int bool;
#define true 1
#define false 0

static int pageFault() {

}

int VMEM_inicia(FILE *log, int tamPag, int tamMem, VMEM_tipoAlgoritmo tipoAlg) {
	unsigned int addr = 0;
	char mode = 'U';
	int res = 0;
	while ((res = fscanf(log, "%x %c ", &addr, &mode)) != EOF) {
		if (res != 2) {
			puts("Erro durante a leitura do arquivo. O arquivo parece estar em formato inválido.");
			return 1;
		}
		printf("Addr: %x Mode: %c\n", addr, mode);
	}
}
