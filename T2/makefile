OBJDIR=./obj

all: simvirtual

# ==== DIRECTORIES ====
mkdir:
	mkdir -p $(OBJDIR)

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
