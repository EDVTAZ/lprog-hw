IDIR =../include
LDIR =../lib
CC=gcc
CFLAGS=-I$(IDIR) -L$(LDIR)

ODIR=obj

LIBS=-lncurses -lrope

_DEPS = buffer.h rope.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o buffer.c
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

editor: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
