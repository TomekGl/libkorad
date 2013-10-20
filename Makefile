CC=gcc
CFLAGS=-I. -Wall
DEPS = libkorad.h
OBJ = libkorad.o korad.o 

all: korad

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

korad: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)


.PHONY: clean

clean:
	rm -f *.o *~ korad 