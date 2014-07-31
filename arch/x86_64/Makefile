# $Id: Makefile 3710 2013-02-24 08:53:52Z michael $

ASM=/usr/bin/nasm

.PHONY: all clean

all:	start32.o header.o start64.o boot_kmain.o serial.o vga.o

start32.o: start32.asm
	$(ASM) -f elf64 start32.asm -o start32.o

header.o: header.asm multiboot2.h
	$(ASM) -f elf64 header.asm -o header.o

start64.o: start64.asm
	$(ASM) -f elf64 start64.asm -o start64.o

boot_kmain.o: boot_kmain.c boot.h types.h
	$(CC) $(CFLAGS) -c boot_kmain.c

serial.o: serial.c boot.h types.h
	$(CC) $(CFLAGS) -c serial.c

vga.o: vga.h vga.c
	$(CC) $(CFLAGS) -c vga.c

clean:
	/bin/rm -rf *.o image
