CFLAGS = -g -Wall -lrt -pthread
CC = gcc

all: chef saladmaker


chef: chef.o parse.o
	$(CC) $(CFLAGS) chef.o parse.o -o chef

saladmaker: saladmaker.o parse.o
	$(CC) $(CFLAGS) saladmaker.o parse.o -o saladmaker

clean:
	rm -f chef
	rm -f saladmaker
	rm -f chef.o
	rm -f saladmaker.o
	rm -f parse.o
	rm -f shared.log
	rm -f saladmaker1.log
	rm -f saladmaker2.log
	rm -f saladmaker3.log