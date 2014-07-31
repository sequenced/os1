# $Id: Makefile 4032 2014-02-18 15:00:00Z michael $

DISTDIR=/home/michael/tmp
# make variables available to sub-make processes by default
export
ROOTDIR=$(CURDIR)
CC=/usr/bin/gcc
LD=/usr/bin/ld
ARCH=x86_64
CFLAGS=-DCONFIG_X86_64 		\
	-I$(ROOTDIR)/lib	\
	-I$(ROOTDIR)		\
	-I.			\
	-fno-builtin		\
	-ffreestanding		\
	-mno-red-zone		\
	-m64			\
	-mcmodel=kernel		\
	-mno-sse		\
	-mno-mmx		\
	-mno-sse2		\
	-mno-3dnow		\
	-no-strict-aliasing	\
	-Wall
unexport SUBDIRS=lib arch/$(ARCH)

.PHONY: all arch lib sys clean

all: arch lib sys image

.ONESHELL:
image:
	find ./lib -type f -name \*.o -exec cp -u {} arch/$(ARCH) \;
	find ./sys -type f -name \*.o -exec cp -u {} arch/$(ARCH) \;
	cd arch/$(ARCH)
	$(LD) -nodefaultlibs -T image.ld -o image *.o

arch:
	cd arch/$(ARCH) && $(MAKE)

lib:
	cd lib && $(MAKE)

sys:
	cd sys && $(MAKE)

clean:
	find . -type f \( -name \*.o -o -name \*.gch \) -delete
	rm -f arch/$(ARCH)/image

iso:	all
	cp -v arch/x86_64/image ~/tmp/iso/boot
	~/bin/grub-mkrescue --output=$(DISTDIR)/grub.iso $(DISTDIR)/iso/

.ONESHELL:
bochs:	iso
	pushd ~/tmp
	bochs -q

