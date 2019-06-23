#include "simvirtual.h"

typedef int bool;
#define true 1
#define false 0

// defines das flags

#define FLAG_READ 0x01
#define FLAG_WRITE 0x02

// estruturas encapsuladas

typedef struct pagina {
	__int64_t lastAccess;
	unsigned char flags;
	int numpag;
} Pagina;

// variáveis encapsuladas

static __int64_t instante = 0;
static int pageFaults = 0;
static int pageHits = 0;

// cabeçalho das funções locais

static int pageFault(VMEM_tipoAlgoritmo tipoAlg, Pagina **mem, int numPaginas, int novaPag);

// funções exportadas

int VMEM_inicia(FILE *log, int tamPag, int tamMem, VMEM_tipoAlgoritmo tipoAlg) {
	int bits = 0;
	int maxAddr = tamPag * 1024 - 1;
	while (maxAddr > 0) {
		maxAddr >>= 1;
		bits++;
	}

	// quantas páginas terão em memória
	int numPaginas = tamMem / tamPag;
	Pagina **mem = (Pagina **) malloc(numPaginas * sizeof(Pagina *));
	if (!mem) {
		puts("Memória insuficiente (erro no malloc do vetor de páginas).");
		return 1;
	}

	// zera as páginas
	for (int i = 0; i < numPaginas; i++) {
		mem[i] = NULL;
	}

	unsigned int addr = 0;
	char mode = 'U';
	int res = 0;
	while ((res = fscanf(log, "%x %c ", &addr, &mode)) != EOF) {
		if (instante % 100 == 0) { // clear de todos os bits R e W
			for (int i = 0; i < numPaginas; i++) {
				if (mem[i] != NULL) {
					mem[i]->flags = 0;
				}
			}
		}

		if (res != 2) {
			puts("Erro durante a leitura do arquivo. O arquivo parece estar em formato inválido.");
			return 1;
		}

		int page = addr >> (32 - bits);
		int vaddr = (addr << bits) >> bits;

		int position = -1;
		for (int i = 0; i < numPaginas; i++) {
			if (mem[i] != NULL && mem[i]->numpag == page) {
				position = i;
				break;
			}
		}

		if (position == -1) { // não achou
			position = pageFault(tipoAlg, mem, numPaginas, page);
			if (position == -1) {
				puts("Memória insuficiente (erro no malloc na pageFault).");
				return 1;
			}
		} else {
			pageHits++;
		}

		mem[position]->lastAccess = instante;
		if (mode == 'R') {
			mem[position]->flags |= FLAG_READ;
		} else {
			mem[position]->flags |= FLAG_WRITE;
		}

		instante++;
	}

	for (int i = 0; i < numPaginas; i++) {
		if (mem[i] != NULL) {
			free(mem[i]);
		}
	}
	free(mem);

	printf("Número de Páginas: %d\n", numPaginas);
	printf("Instante Final: %ld\n", instante);
	printf("Page faults: %d\n", pageFaults);
	printf("Page hits: %d\n", pageHits);

	return 0;
}

// funções locais

// retorna o índice na memória com a nova página
static int pageFault(VMEM_tipoAlgoritmo tipoAlg, Pagina **mem, int numPaginas, int novaPag) {
	pageFaults++;

	int pag = 0;
	int lastAccessed = instante;
	for (int i = 0; i < numPaginas; i++) {
		if (mem[i] == NULL) { // achou lugar vazio pra nova página
			pag = i;
			break;
		}

		// TIPO DE ALGORÍTMO LRU
		if (tipoAlg == ALG_LRU && mem[i]->lastAccess < lastAccessed) {
			lastAccessed = mem[i]->lastAccess;
			pag = i;
		}

		// TIPO DE ALGORÍTMO NRU

	} // sai do for com pag = número da página a ser substituída

	if (mem[pag] != NULL) free(mem[pag]); // "tira da memória"
	mem[pag] = (Pagina *) malloc(sizeof(Pagina)); // "coloca nova na memória"
	if (mem[pag] == NULL) return -1;

	mem[pag]->flags = 0;
	mem[pag]->lastAccess = 0;
	mem[pag]->numpag = novaPag;

	return pag;
}
