CFLAGS = -g -Wall 
CC = gcc

all: quic


quic: main.o core.o utilities.o
	$(CC) $(CFLAGS) main.o core.o utilities.o -o quic

clean:
	rm *.o
	rm quic
