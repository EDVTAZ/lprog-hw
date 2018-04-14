IDIR =../include
LDIR =../lib
CC=gcc
CFLAGS=-I$(IDIR) -L$(LDIR) -ggdb

ODIR=obj

LIBS=-lncurses -lrope -lmsgpackc

_DEPS = ui.h buffer.h rope.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o ui.o buffer.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

editor: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
