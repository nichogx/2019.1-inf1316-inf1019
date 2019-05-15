#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>

#ifndef SEMAFORO_H
#define SEMAFORO_H

// inicializa o valor do semáforo
int setSemValue(int semId);

// remove o semáforo
void delSemValue(int semId);

// operação P
int semaforoP(int semId);

//operação V
int semaforoV(int semId);

#endif // SEMAFORO_H
