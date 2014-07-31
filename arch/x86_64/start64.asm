[BITS 64]
%define STACKSZ 0x2000
extern boot_kmain
section .text
global _start64
_start64:
	jmp	_Lclr		; empty pre-fetch queue
_Lclr:
	;; setup temporary 64-bit stack
	mov	rsp, _Lestack64
	mov	rbp, rsp

	;; x86_64 calling convention
	;; rdi, rsi, rdx ...
	mov	rdi, rax
	mov	rsi, rbx

	xor	rax, rax
	mov	ds, rax
	mov	ss, rax
	mov	es, rax
	mov	fs, rax
	mov	gs, rax

	call boot_kmain

	jmp	$

section .data
_Lstack64:
	times	STACKSZ db 0
_Lestack64:
