OBJDIR=./obj

all: simvirtual

# ==== DIRECTORIES ====

$(OBJDIR)/%.o: %.c
	mkdir -p $(OBJDIR)
	gcc -c -o $@ $<

# ==== PROGRAM ====

OBJS=$(addprefix $(OBJDIR)/, main.o simvirtual.o)
simvirtual: $(OBJS) simvirtual.h
	gcc -Wall -o simvirtual $(OBJS)

# ==== CLEAN ====

clean:
	rm -f simvirtual
	rm -rf $(OBJDIR)

# ==== TEST MEMORY ====

testmem:
	valgrind --leak-check=full --track-origins=yes ./simvirtual lru compilador.log 8 128
	valgrind --leak-check=full --track-origins=yes ./simvirtual nru compilador.log 8 128
	valgrind --leak-check=full --track-origins=yes ./simvirtual novo compilador.log 8 128
