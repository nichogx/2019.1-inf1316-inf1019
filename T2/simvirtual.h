#include <stdlib.h>
#include <stdio.h>

typedef enum {
	ALG_LRU,
	ALG_NRU,
	ALG_NOVO
} VMEM_tipoAlgoritmo;

/**
 * Inicia a simulação de memória virtual.
 *
 * Retorna 0 caso tudo tenha corrido normalmente, 1 em caso de erro.
 */
int VMEM_inicia(FILE *log, int tamPag, int tamMem, VMEM_tipoAlgoritmo);
