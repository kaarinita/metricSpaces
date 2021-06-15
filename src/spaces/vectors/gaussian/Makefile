CFLAGS = -Wall -c -O2 $(DEFINE) -o $@

all: gaussora

gaussora: gaussora.o
	gcc -o gaussora gaussora.o -lm

gaussora.o: gaussora.c
	gcc $(CFLAGS) -c gaussora.c 
