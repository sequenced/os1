.PHONY: all clean mm

all: console.o mkdev.o errno.o mm

mm:
	cd mm && $(MAKE)

console.o: console.c console.h
	$(CC) $(CFLAGS) -c console.c

mkdev.o: mkdev.c dev.h
	$(CC) $(CFLAGS) -c mkdev.c

errno.o: errno.c errno.h
	$(CC) $(CFLAGS) -c errno.c

clean:
	/bin/rm -rf *.o
