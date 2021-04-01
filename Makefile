# An admittedly primitive Makefile
# To compile, type "make" or make "all"
# To remove files, type "make clean"

CC = gcc
CFLAGS = -Wall -pthread
OBJS = wserver.o wclient.o request.o io_helper.o cda.o integer.o
OOPTS = -Wall -Wextra -g -c
EXEC_FLAGS = -s SFF

.SUFFIXES: .c .o 

all: wserver wclient spin.cgi

wserver: wserver.o request.o io_helper.o cda.o integer.o
	$(CC) $(CFLAGS) -o wserver wserver.o request.o io_helper.o cda.o integer.o

wclient: wclient.o io_helper.o
	$(CC) $(CFLAGS) -o wclient wclient.o io_helper.o

spin.cgi: spin.c
	$(CC) $(CFLAGS) -o spin.cgi spin.c

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

test: wserver
	./wserver $(EXEC_FLAGS)

cda.o : cda.c cda.h
		gcc $(OOPTS) cda.c

integer.o : integer.c integer.h
		gcc $(OOPTS) integer.c

clean:
	-rm -f $(OBJS) wserver wclient spin.cgi
