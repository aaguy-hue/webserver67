IDIR =../include
CC=gcc
CFLAGS= -Wall -Werror -Wextra -g -I$(IDIR) -lyaml

ODIR=obj
LDIR =../lib

LIBS=-lm

_DEPS = hashmap.h fields.h control_data.h util.h request.h config.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = server.o hashmap.o fields.o control_data.o util.o config.o
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
