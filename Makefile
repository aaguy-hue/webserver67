IDIR =./include
CC=gcc
CFLAGS= -Wall -Werror -Wswitch -Wimplicit-fallthrough -Wextra -g -I$(IDIR) -lcyaml -lyaml -lz -pedantic -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=700
# -D_XOPEN_SOURCE=700 is needed for sigaction() and sigfillset() since they're not in the default C standard, and -D_DEFAULT_SOURCE is needed for strncat() since it's not in the default C standard either


ODIR=obj
LDIR =./lib

LIBS=-lm

_DEPS = config.h fields.h fileutil.h hashmap.h headers.h request.h response.h startline.h util.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = server.o config.o fields.o fileutil.o hashmap.o headers.o request.o response.o startline.o util.o
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

install: build/server.out
	cp build/server.out /usr/local/bin/ws67
