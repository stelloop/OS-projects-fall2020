# a simple makefile to start with :)
 

MODULES = ./modules
INCLUDES = ./includes

CC = gcc
CFLAGS = -Wall -g -pg -Wextra -I$(INCLUDES)

PROGRAM = mngstd

OBJS = mngstd.o parse.o init.o queries.o $(MODULES)/list.o $(MODULES)/hashtable.o $(MODULES)/compare_funcs.o

$(PROGRAM): $(OBJS)
	$(CC) $(OBJS) -o $(PROGRAM) 
	