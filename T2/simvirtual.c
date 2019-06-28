#ifdef _DEBUG
#include <signal.h>
#endif

#include "simvirtual.h"

// tipo booleano

typedef int bool;
#define true 1
#define false 0

// defines das flags

#define FLAG_NONE 0x00
#define FLAG_READ 0x01
#define FLAG_WRITE 0x02

// constantes

#define TAM_VET_FUT 1000
#define INST_NOVO 128
#define INST_NRU 1000

// estruturas encapsuladas

typedef struct entrTab {
	__int64_t lastAccess;
	unsigned int flagsRW; // 0001 READ, 0010 WRITE
	unsigned int endFisico;
	bool inMemory;
} EntradaTabela;

// variáveis encapsuladas

static __int64_t instante = 0;
static int pageFaults = 0;
static int pageHits = 0;
static int memWrites = 0;

#ifdef _DEBUG
// variáveis globais para liberar memória
static EntradaTabela **globalTP = NULL;
static int *globalMEM = NULL;
static int globalNPages = 0;
#endif

// cabeçalho das funções locais

static void pageFault(VMEM_tipoAlgoritmo tipoAlg, int *mem, int numQuadros, EntradaTabela **tp, int numPag, int* futuro);
static int power2(int num);

#ifdef _DEBUG
static void finalize(int signal);
#endif

// funções exportadas

int VMEM_inicia(FILE *log, int tamPag, int tamMem, VMEM_tipoAlgoritmo tipoAlg) {
	tamMem *= 1024; // MB para KB

	#ifdef _DEBUG
	signal(SIGINT, finalize);
	#endif

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

	#ifdef _DEBUG
	// globaliza tp ao módulo para liberar
	globalTP = tp;
	globalNPages = numPages;
	#endif

	// preenche tabela de paginas
	for (int i = 0; i < numPages; i++) {
		tp[i] = (EntradaTabela *) malloc(sizeof(EntradaTabela));
		tp[i]->endFisico = 0;
		tp[i]->flagsRW = FLAG_NONE;
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

	// vetor de páginas lidas para algoritmo NOVO
	int *futuro = NULL;
	if (tipoAlg == ALG_NOVO) {
		futuro = (int *) malloc(TAM_VET_FUT * sizeof(int));
		if (!futuro) {
			puts("Memória insuficiente (erro no malloc do vetor futuro).");
			return 1;
		}
	}

	#ifdef _DEBUG
	// globaliza mem ao módulo para liberar
	globalMEM = mem;
	#endif

	// desaloca a memória toda
	for (int i = 0; i < numQuadros; i++) {
		mem[i] = -1;
	}

	#ifdef _DEBUG
	puts("Aperte enter para ir de linha em linha.\n");
	#endif

	unsigned int addr = 0;
	char mode = 'U';
	int res = 0;
	while ((res = fscanf(log, "%x %c ", &addr, &mode)) != EOF) {
		if (res != 2) {
			puts("Erro durante a leitura do arquivo. O arquivo parece estar em formato inválido.");
			return 1;
		}

		// reseta as flags a cada INST_NRU iterações (só se for NRU)
		if (tipoAlg == ALG_NRU && instante % INST_NRU == 0) {
			for (int i = 0; i < numQuadros; i++) {
				if (mem[i] != -1 && (tp[mem[i]]->flagsRW & FLAG_READ)) tp[mem[i]]->flagsRW &= FLAG_WRITE;
			}
		}

		// atualiza o vetor novo a cada INST_NOVO iterações (só se for NOVO)
		if (tipoAlg == ALG_NOVO && instante % INST_NOVO == 0) {

			int endOfFile = 0;
			fseek(log, TAM_VET_FUT - INST_NOVO, SEEK_CUR);

			for (int i = 0; i < TAM_VET_FUT; i++) {

				// anda com as informações do vetor sem precisar ler o arquivo
				if(i < TAM_VET_FUT - INST_NOVO) {
					futuro[i] = futuro[i + INST_NOVO];

				} else {
					int resAux = 0;
					char modeAux = 'U';

					// atualiza as últimas INST_NOVO posições com fscanf do arquivo
					if (endOfFile == 0 && (resAux = fscanf(log, "%x %c ", &futuro[i], &modeAux)) == EOF) {
						endOfFile = i;
						futuro[i] = -1;
					} else {
						futuro[i] = -1;
					}
				}
			}

			fseek(log, -TAM_VET_FUT + endOfFile, SEEK_CUR);

		}

		unsigned int page = addr >> bits;

		#ifdef _DEBUG
		unsigned int vaddr = (addr << (32 - bits)) >> (32 - bits);
		printf("LIDO: %08X %c\n", addr, mode);
		printf("page %d desl %u\n", page, vaddr);
		#endif

		if (!tp[page]->inMemory) { // não achou
			pageFault(tipoAlg, mem, numQuadros, tp, page, futuro);
		} else {
			pageHits++;
		}

		tp[page]->lastAccess = instante;
		tp[page]->flagsRW |= FLAG_READ;
		if (mode == 'W') {
			tp[page]->flagsRW |= FLAG_WRITE;
		}

		instante++;

		#ifdef _DEBUG
		getc(stdin);
		#endif
	}

	// libera tabela de páginas
	for (int i = 0; i < numPages; i++) {
		free(tp[i]);
	}
	free(tp);

	// libera vetor para ALG_NOVO
	if (tipoAlg == ALG_NOVO) free(futuro);

	// libera vetor que representa memória
	free(mem);

	printf("Número de Páginas em Memória: %d\n", numQuadros);
	printf("Número de Páginas na TP: %d\n", numPages);
	printf("Instante Final: %ld\n", instante);
	printf("Page faults: %d\n", pageFaults);
	printf("Page hits: %d\n", pageHits);
	printf("Reescritas para a memória: %d\n", memWrites);

	return 0;
}

// funções locais

/**
 * tipoAlg -> o tipo do algorítmo
 * mem -> o vetor que representa a memória
 * numQuadros -> o tamanho do vetor mem
 * tp -> a tabela de páginas
 * numPag -> o tamanho da tp
 * futuro -> o vetor de páginas futuras (tamanho TAM_VET_FUT)
 */
static void pageFault(VMEM_tipoAlgoritmo tipoAlg, int *mem, int numQuadros, EntradaTabela **tp, int numPag, int* futuro) {
	pageFaults++;

	#ifdef _DEBUG
	puts("PAGE FAULT!");
	#endif

	int newEnd = 0; // novo endereço da página na memória
	bool found = false;
	for (int i = 0; i < numQuadros && !found; i++) {
		if (mem[i] == -1) { // achou lugar vazio pra nova página na memória
			newEnd = i;
			found = true;
		}
	}

	if (!found) {

		if (tipoAlg == ALG_LRU) { // TIPO DE ALGORÍTMO LRU
			int lastAccessed = instante;
			for (int i = 0; i < numQuadros; i++) {
				if(tp[mem[i]]->lastAccess < lastAccessed) {
					lastAccessed = tp[mem[i]]->lastAccess;
					newEnd = i;
				}
			}
		} else if (tipoAlg == ALG_NRU) { // TIPO DE ALGORÍTMO NRU
			bool nruFound = false;

			// busca --
			for (int i = 0; i < numQuadros && !nruFound; i++) {
				if (!(tp[mem[i]]->flagsRW & FLAG_READ) && !(tp[mem[i]]->flagsRW & FLAG_WRITE)) {
					// achou um sem read e sem write
					newEnd = i;
					nruFound = true;
				}
			}

			// busca -W
			for (int i = 0; i < numQuadros && !nruFound; i++) {
				if (!(tp[mem[i]]->flagsRW & FLAG_READ)) {
					// achou um sem read e com write
					newEnd = i;
					nruFound = true;
				}
			}

			// busca R-
			for (int i = 0; i < numQuadros && !nruFound; i++) {
				if (!(tp[mem[i]]->flagsRW & FLAG_WRITE)) {
					// achou um sem write, com read
					newEnd = i;
					nruFound = true;
				}
			}

			// se chegou aqui sem achar, todos são RW. Tirar o primeiro. (já é zero aqui)
		} else if (tipoAlg == ALG_NOVO) { // TIPO DE ALGORÍTMO NOVO

		}

	} // sai daqui com newEnd = endereço da página a ser substituída

	if (mem[newEnd] != -1) {// "tira da memória", não era um endereço vazio
		tp[mem[newEnd]]->inMemory = false;
		if (tp[mem[newEnd]]->flagsRW & FLAG_WRITE) {
			tp[mem[newEnd]]->flagsRW = FLAG_NONE;
			memWrites++;
		}
	}

	tp[numPag]->inMemory = true; // "coloca nova na memória"
	tp[numPag]->endFisico = newEnd;

	mem[newEnd] = numPag;
}

/**
 * retorna 2^num
 */
static int power2(int num) {
	int result = 1;
	while (num) {
		result *= 2;
		num--;
	}

	return result;
}

#ifdef _DEBUG
// trata liberação de memória se for chamado com ^C
static void finalize(int signal) {
	puts("\n[SIGINT] Liberando memória...");
	// libera tabela de páginas
	for (int i = 0; i < globalNPages; i++) {
		free(globalTP[i]);
	}
	free(globalTP);

	// libera vetor que representa memória
	free(globalMEM);
	puts("[SIGINT] Finalizado.");
	exit(1);
}
#endif
