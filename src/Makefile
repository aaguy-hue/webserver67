IDIR =../include
CC=gcc
CFLAGS= -Wall -Werror -Wextra -g -I$(IDIR)

ODIR=obj
LDIR =../lib

LIBS=-lm

_DEPS = hashmap.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = server.o hashmap.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/%.o: $(LDIR)/%.c $(DEPS) | $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

build/server.out: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
