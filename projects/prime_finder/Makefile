CC = gcc
FLAGS = -Wall 
TARGETS = myprime inner leaf
OBJS = myprime.o list.o inner.o leaf.o


all:$(TARGETS)

myprime:myprime.o inner.o leaf.o list.o read_write.o
	$(CC) $(FLAGS) -o myprime myprime.o  list.o read_write.o

inner:inner.o list.o read_write.o
		$(CC) $(FLAGS) -o inner inner.o  list.o read_write.o -lm

leaf:leaf.o read_write.o
	$(CC) $(FLAGS) -o leaf leaf.o read_write.o -lm

list.o:
	$(CC) $(FLAGS) -o list.o -c list.c

read_write.o:
	$(CC) $(FLAGS) -o read_write.o -c read_write.c
