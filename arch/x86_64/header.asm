	;; Multiboot header
	;; implements Multiboot 1.6 specification (aka `multiboot2`)
[BITS 32]
%define	MULTIBOOT_HEADER_ALIGN 8
%define MULTIBOOT2_HEADER_MAGIC 0xe85250d6
%define MULTIBOOT_HEADER_TAG_END 0
%define MULTIBOOT_HEADER_TAG_ADDRESS 2
%define MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS 3
%define MULTIBOOT_ARCHITECTURE_I386 0
%define MULTIBOOT_HEADER_TAG_OPTIONAL 1
	
extern _bootstrap32, _start32
section .data
	
	;; typically 8 byte-aligned
	align	MULTIBOOT_HEADER_ALIGN
Lmboot_hdr:
	;; magic
	dd	MULTIBOOT2_HEADER_MAGIC
	;; architecture
	dd	MULTIBOOT_ARCHITECTURE_I386
	;; header length
	dd	(Lemboot_hdr-Lmboot_hdr)
	;; checksum
	dd	-(MULTIBOOT2_HEADER_MAGIC+0+(Lemboot_hdr-Lmboot_hdr))

	;; tags...
	dw	MULTIBOOT_HEADER_TAG_ADDRESS
	dw	MULTIBOOT_HEADER_TAG_OPTIONAL
	dd	24
	dd	Lmboot_hdr
	dd	_bootstrap32
	dd	0x0000_0000	; load entire image file
	dd	0x0000_0000	; assume no BSS section

	align	MULTIBOOT_HEADER_ALIGN
	
	dw	MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS
	dw	MULTIBOOT_HEADER_TAG_OPTIONAL
	dd	12
	dd	_start32

	align	MULTIBOOT_HEADER_ALIGN
	
	dw	MULTIBOOT_HEADER_TAG_END
	dw	MULTIBOOT_HEADER_TAG_OPTIONAL
	dd	8
Lemboot_hdr:	
