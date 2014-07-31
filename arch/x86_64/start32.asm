	;; os1 loader (multiboot2 compatible)
	;; assumes protected mode, sets up long mode
[BITS 32]
extern _boot_hat

%define PAGE.ADDR	0xffffffff_fffff000
%define PAGE.P		(1)
%define PAGE.RW		(1<<1)
%define CR4.PAE		(0x1<<5)
%define CR0.PG		(0x1<<31)
%define EFER.LME	(0x1<<8)
%define SEG		0x10
%define STACKSZ		0x200

	;; paging structures 4-KByte aligned, contiguous
%define PML4	_boot_hat	 ; PML4E linear address
%define PDPT	_boot_hat+0x1000 ; Page Directory Pointer Table linear address
%define PD	_boot_hat+0x2000 ; Page Directory linear address
%define PT	_boot_hat+0x3000 ; Page Table (1st 2MB) linear address
	
extern _start64
section .text
global _start32
_start32:
	cli

	;; setup temporary 32-bit stack
	mov	esp, _Lestack32	; setup stack (grows down)
	push	0x0
	popf			; clear flags

	push	ebx		; save multiboot2 pointers
	push	eax
	
	;; identity map first 2MB of physical space
	;; PML4[0]->PDPT[0]->PD[0]->PT[0-511]->0x000000-0x200000

	;; clear PML4, PDPT, PD and PT
	lea	edi, [es:_boot_hat]
	xor	eax, eax
	mov	ecx, 0x1000	; 4 * 512 8-byte entries, 4-byte increment
	rep	stosd
	
	mov	eax, PML4
	and	eax, PAGE.ADDR
	mov	cr3, eax
	lea	edi, [es:PML4]
	mov	[edi], dword PDPT
	and	[edi], dword PAGE.ADDR
	or	[edi], dword PAGE.P|PAGE.RW

	lea	edi, [es:PDPT]
	mov	[edi], dword PD
	and	[edi], dword PAGE.ADDR
	or	[edi], dword PAGE.P|PAGE.RW
	
	lea	edi, [es:PD]
	mov	[edi], dword PT
	and	[edi], dword PAGE.ADDR
	or	[edi], dword PAGE.P|PAGE.RW

	lea	edi, [es:PT]
	mov	ebx, PAGE.P|PAGE.RW
	mov	ecx, 0x200	; 512 entries
_Lpt:	
	mov	[edi], ebx
	add	edi, 0x8	; next PTE
	add	ebx, 0x1000	; next page
	loop	_Lpt

	;; Switch to IA-32e mode

	mov	eax, cr4
	or	eax, CR4.PAE
	mov	cr4, eax

	mov	ecx, 0xc0000080
	rdmsr
	or	eax, EFER.LME
	wrmsr
	
	mov	eax, cr0
	or	eax, CR0.PG
	mov	cr0, eax

	pop	eax
	pop	ebx

	;; Switch to IA-32e long mode (via far jump)
	lgdt	[_Lgdt.ptr]
	jmp	SEG:_start64	; reloads CS, interprets L bit in GDT

	;; Global Descriptor Table

_Lgdt:
.null:
	dq	0x00000000_00000000 ; first descriptor not used
.code:
	dq	0x00209800_00000000 ; L, P set
.data:
	dq	0x00209800_00000000 ; L, P set
.ptr:
	dw	$-_Lgdt-1	; limit (table size in bytes)
	dq	_Lgdt		; base address

section .data
_Lstack32:
	times	STACKSZ db 0
_Lestack32:
