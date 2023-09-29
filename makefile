CC=gcc
CFLAGS=-I.
DEPS = ext.h fat.h values.h
OBJ = main.o ext.o fat.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

shooter: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)