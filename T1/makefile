all: progfilho escalonador

progfilho: progfilho.c
	gcc -Wall -o progfilho progfilho.c

escalonador: escalonador.c fila.c fila.h semaforo.c semaforo.h
	gcc -Wall -o escalonador escalonador.c fila.c semaforo.c -pthread

clean:
	rm escalonador progfilho
