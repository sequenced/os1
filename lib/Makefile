# $Id: Makefile 3689 2013-02-17 06:36:03Z michael $

.PHONY: all clean

all:	printk.o

printk.o: printk.h printk.c
	$(CC) $(CFLAGS) -c printk.c

clean:
	/bin/rm -rf *.o
