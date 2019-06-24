#include "simvirtual.h"

typedef int bool;
#define true 1
#define false 0

// defines das flags

#define FLAG_READ 0x01
#define FLAG_WRITE 0x02

// estruturas encapsuladas

typedef struct entrTab {
	__int64_t lastAccess;
	unsigned char flagsRW; // 0001 READ, 0010 WRITE
	unsigned int endFisico;
	bool inMemory;
} EntradaTabela;

// variáveis encapsuladas

static __int64_t instante = 0;
static int pageFaults = 0;
static int pageHits = 0;

// cabeçalho das funções locais

static void pageFault(VMEM_tipoAlgoritmo tipoAlg, int *mem, int numQuadros, EntradaTabela **tp, int numPag);
static int power2(int num);

// funções exportadas

int VMEM_inicia(FILE *log, int tamPag, int tamMem, VMEM_tipoAlgoritmo tipoAlg) {
	unsigned int bits = 0;
	unsigned int maxAddr = tamPag * 1024 - 1;
	while (maxAddr > 0) {
		maxAddr >>= 1;
		bits++;
	}

	// tabela de páginas
	int numPages = power2(32 - bits);
	EntradaTabela **tp = (EntradaTabela **) malloc(numPages * sizeof(EntradaTabela *));
	if (!tp) {
		puts("Memória insuficiente (erro no malloc da tabela de páginas).");
		return 1;
	}

	// preenche tabela de paginas
	for (int i = 0; i < numPages; i++) {
		tp[i] = (EntradaTabela *) malloc(sizeof(EntradaTabela));
		tp[i]->endFisico = 0;
		tp[i]->flagsRW = 0;
		tp[i]->inMemory = false;
		tp[i]->lastAccess = 0;
	}

	// quantas páginas terão em memória
	int numQuadros = tamMem / tamPag;
	int *mem = (int *) malloc(numQuadros * sizeof(int));
	if (!mem) {
		puts("Memória insuficiente (erro no malloc do vetor que representa memória).");
		return 1;
	}

	// desaloca a memória toda
	for (int i = 0; i < numQuadros; i++) {
		mem[i] = -1;
	}

	unsigned int addr = 0;
	char mode = 'U';
	int res = 0;
	while ((res = fscanf(log, "%x %c ", &addr, &mode)) != EOF) {
		if (res != 2) {
			puts("Erro durante a leitura do arquivo. O arquivo parece estar em formato inválido.");
			return 1;
		}

		unsigned int page = addr >> bits;
		unsigned int vaddr = (addr << (32 - bits)) >> (32 - bits);

		if (page > numPages) {
			puts("Erro durante a leitura do arquivo. O número da página é maior que o número de páginas");
			return 1;
		}

		if (!tp[page]->inMemory) { // não achou
			pageFault(tipoAlg, mem, numQuadros, tp, page);
		} else {
			pageHits++;
		}

		tp[page]->lastAccess = instante;
		if (mode == 'R') {
			tp[page]->flagsRW |= FLAG_READ;
		} else {
			tp[page]->flagsRW |= FLAG_WRITE;
		}

		instante++;
	}

	// libera tabela de páginas
	for (int i = 0; i < numPages; i++) {
		free(tp[i]);
	}
	free(tp);

	// libera vetor que representa memória
	free(mem);

	printf("Número de Páginas em Memória: %d\n", numQuadros);
	printf("Número de Páginas na TP: %d\n", numPages);
	printf("Instante Final: %ld\n", instante);
	printf("Page faults: %d\n", pageFaults);
	printf("Page hits: %d\n", pageHits);

	return 0;
}

// funções locais

// retorna o índice na memória com a nova página
static void pageFault(VMEM_tipoAlgoritmo tipoAlg, int *mem, int numQuadros, EntradaTabela **tp, int numPag) {
	pageFaults++;

	int newEnd = 0; // novo endereço da página na memória
	int lastAccessed = instante;
	for (int i = 0; i < numQuadros; i++) {
		if (mem[i] == -1) { // achou lugar vazio pra nova página na memória
			newEnd = i;
			break;
		}

		// TIPO DE ALGORÍTMO LRU
		if (tipoAlg == ALG_LRU && tp[mem[i]]->lastAccess < lastAccessed) {
			lastAccessed = tp[mem[i]]->lastAccess;
			newEnd = i;
		}

		// TIPO DE ALGORÍTMO NRU

	} // sai do for com pag = endereço da página a ser substituída

	if (mem[newEnd] != -1) {// "tira da memória", não era um endereço vazio
		tp[mem[newEnd]]->inMemory = false;
	}

	tp[numPag]->inMemory = true; // "coloca nova na memória"
	tp[numPag]->endFisico = newEnd;

	mem[newEnd] = numPag;
}

// retorna a potência de 2 equivalente
static int power2(int num) {
	int result = 1;
	while (num) {
		result *= 2;
		num--;
	}

	return result;
}
