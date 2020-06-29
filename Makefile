PREFIX = /usr/local

IDIR = /usr/include/SDL2
CC=gcc
CFLAGS=-I$(IDIR)

ODIR=src

LIBS=-lSDL2

_DEPS = main.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

all: chemu clean

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

chemu: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

install: chemu
	mkdir -p $(PREFIX)/bin
	cp chemu $(PREFIX)/bin/chemu
	chmod +x $(PREFIX)/bin/chemu

clean:
	rm -f $(ODIR)/*.o

.PHONY: clean install
