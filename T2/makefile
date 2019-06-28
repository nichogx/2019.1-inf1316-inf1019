# ==== INIT ====

OBJDIR = ./obj
FLAGS =

# ==== GENERAL ====

all: simvirtual

remake: clean all

debug: FLAGS += -D_DEBUG -g
debug: all

# ==== OBJECTS ====

$(OBJDIR)/%.o: %.c
	mkdir -p $(OBJDIR)
	gcc -Wall -c -o $@ $< $(FLAGS)

# ==== PROGRAM ====

OBJS = $(addprefix $(OBJDIR)/, main.o simvirtual.o)
simvirtual: $(OBJS) simvirtual.h
	gcc -Wall -o simvirtual $(OBJS) -pthread

# ==== CLEAN ====

clean:
	rm -f simvirtual
	rm -rf $(OBJDIR)
