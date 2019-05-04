all: progfilho escalonador

progfilho: progfilho.c
	gcc -Wall -o progfilho progfilho.c

escalonador: escalonador.c fila.c fila.h
	gcc -Wall -o escalonador escalonador.c fila.c

clean:
	rm escalonador progfilho
