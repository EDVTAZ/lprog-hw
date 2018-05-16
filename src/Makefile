IDIR =../include
LDIR =../lib
CC=gcc
CFLAGS=-I$(IDIR) -L$(LDIR) -ggdb

ODIR_S=obj/server
ODIR_E=obj/editor

LIBS=-lncurses -lrope

_DEPS = ui.h buffer.h rope.h parson.h msg.h utilities.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ_E = editor.o ui.o buffer.o parson.o msg.o utilities.o
OBJ_E = $(patsubst %,$(ODIR_E)/%,$(_OBJ_E))

_OBJ_S = server.o ui.o buffer.o parson.o msg.o utilities.o
OBJ_S = $(patsubst %,$(ODIR_S)/%,$(_OBJ_S))

$(ODIR_S)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR_E)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: server editor

server: $(OBJ_S)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

editor: $(OBJ_E)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR_E)/*.o $(ODIR_S)/*.o *~ core $(INCDIR)/*~ 

