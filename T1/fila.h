#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef FILA_H
#define FILA_H

typedef struct filaPid FilaPid;

// cria e retorna uma fila de pids
FilaPid *FPID_create();

// destroi uma fila de pids
void FPID_destroy(FilaPid *fila);

// enfilera um pid (retorna 0 em caso de erro, 1 em caso de sucesso)
int FPID_enqueue(pid_t pid, FilaPid *fila);

// desenfilera um pid (retorna o pid ou -1 em caso de erro)
pid_t FPID_dequeue(FilaPid *fila);

// checa se uma fila de pids é vazia
int FPID_isempty(FilaPid *fila);

// printa uma fila inteira
void FPID_print(FilaPid *fila, int num);

#endif // FILA_H
