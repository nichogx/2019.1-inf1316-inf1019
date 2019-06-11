#include "fila.h"

typedef struct no {
	pid_t pid;
	struct no *next;
} No;

struct filaPid {
	No *ini;
};

FilaPid *FPID_create() {
	FilaPid *fl = (FilaPid *) malloc(sizeof(FilaPid));
	if (!fl) return NULL;

	fl->ini = NULL;

	return fl;
}

void FPID_destroy(FilaPid *fila) {
	No *atual = fila->ini;

	while (atual) {
		No *ant = atual;
		atual = atual->next;

		free(ant);
	}

	free(fila);
}

int FPID_enqueue(pid_t pid, FilaPid *fila) {
	No *newNo = (No *) malloc(sizeof(No));
	if (!newNo) return 0;

	newNo->next = NULL;
	newNo->pid = pid;

	if (!fila->ini) {
		fila->ini = newNo;
	} else {
		No *p = fila->ini;
		while (p->next) {
			p = p->next;
		}

		p->next = newNo;
	}

	return 1;
}

pid_t FPID_dequeue(FilaPid *fila) {
	if (!fila->ini) return -1;

	pid_t ret = fila->ini->pid;

	No *fr = fila->ini;
	fila->ini = fila->ini->next;

	free(fr);

	return ret;
}

int FPID_isempty(FilaPid *fila) {
	return fila->ini == NULL;
}

void FPID_print(FilaPid *fila, int num) {
	No *p = fila->ini;

	printf("FILA %d: ", num);
	while (p != NULL) {
		printf("%d -> ", p->pid);
		p = p->next;
	}

	puts("NULL");
}
