; vim:set tabstop=14:

; When we get to start we are in 32-bit protected mode,
;
; eax has to be 0x36d76289
; ebx has to be the 32bit physical address of the multiboot information structure
; cs  must be a 32bit read/execute code segment with an offset of 0x0 and limit of 0xffffffff
; ds
; es
; fs
; gs
; ss must be a 32bit read/write data segment with an offset of 0x0 and limit of 0xffffffff
; A20 gate is enabled
; CR0 bit 31 (PG) cleared, bit 0 (PE) set; other bits undefined.
; EFLAGS bit 17(VM) cleared, bit 9(IF) cleared; other bits undefined
; all other registers are undefined.

; TODO(peter): use macros for these flags/bitsets

global	start
extern	longmodeStart

	section .text
	[BITS 32]

start:	mov	esp, stack
	mov	edi, ebx

	call	checkMB
	call	checkCPUID
	call	checkLM

	call	setupPaging
	call	enablePaging
;	call	setupSSE

	lgdt	[gdt64.pointer]

	mov	ax, gdt64.data
	mov	ss, ax
	mov	ds, ax
	mov	es, ax


; first six arguments are passed in: RDI, RSI, RDX, RCX, R8, R9

	jmp	gdt64.code:longmodeStart

;======================================[ Check Multiboot magic ]===
checkMB:	cmp	eax, 0x36d76289
	jne	.noMultiboot
	ret
.noMultiboot:	mov	al, "0"
	jmp	error

;======================================[ Check CPUID ]===
checkCPUID:	pushfd
	pop	eax
	mov	ecx, eax
	xor	eax, 1 << 21	; Poke the ID-bit

	push	eax
	popfd

	pushfd
	pop	eax

	push	ecx
	popfd

	cmp	eax, ecx
	je	.noCPUID
	ret

.noCPUID:	mov	al, "1"
	jmp	error

;======================================[ Check longmode ]===
checkLM:	mov	eax, 0x80000000
	cpuid
	cmp	eax, 0x80000001
	jb	.noLongmode

	mov	eax, 0x80000001
	cpuid
	test	edx, 1 << 29
	jz	.noLongmode
	ret

.noLongmode	mov	al, "2"
	jmp	error

;======================================[ Setup SSE ]===
setupSSE:	mov	eax, 0x1
	cpuid
	test	edx, 1 << 25
	jz	.noSSE

	mov	eax, cr0
	and	ax, 0xfffb
	or	ax, 0x2
	mov	cr0, eax
	mov	eax, cr4
	or	ax, 3 << 9
	mov	cr4, eax
	ret

.noSSE:	mov	al, "a"
	jmp	error

;======================================[ Setup Paging ]===
setupPaging:	mov	eax, p4
	or	eax, 0b11
	mov	[p4 + 511*8], eax

	mov	eax, p3
	or	eax, 0b11
	mov	[p4], eax

	mov	eax, p2
	or	eax, 0b11
	mov	[p3],  eax

	mov	ecx, 0
.mapP2:	mov	eax, 0x20000
	mul	ecx
	or	eax, 0b10000011
	mov	[p2 + ecx * 8], eax

	inc	ecx
	cmp	ecx, 512
	jne	.mapP2
	ret

;======================================[ Enable Paging ]===
enablePaging:	mov	eax, p4
	mov	cr3, eax

	mov	eax, cr4
	or	eax, 1 << 5
	mov	cr4, eax

	mov	ecx, 0xc0000080
	rdmsr
	or	eax, 1 << 8
	wrmsr

	mov	eax, cr0
	or	eax, 1 << 31
	mov	cr0, eax
	ret

;======================================[ Error ]===
; parameter: error code (ascii) in  al.
;
error:	mov dword	[0xb8000], 0x4f524f45
	mov dword	[0xb8004], 0x4f3a4f52
	mov dword	[0xb8008], 0x4f204f20
	mov	[0xb800a], al
	hlt


	section	.bss

	align 4096
p4:	resb	4096
p3:	resb	4096
p2:	resb	4096
stackBottom:	resb	4096*2
stack:

	section	.rodata

gdt64:	dq	0
.code	equ	$ - gdt64
	dq	(1 << 44) | (1 << 47) | (1 << 41) | (1 << 43) | (1 << 53)
.data	equ	$ - gdt64
	dq	(1 << 44) | (1 << 47) | (1 << 41)
.pointer	dw	$ - gdt64 - 1
	dq	gdt64
