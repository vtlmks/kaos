	[bits 16]
	[section .text]
	[Org 0x8000]

struc	MemInfo
.baseAddrLow	resd	1
.baseAddrHi	resd	1
.lengthLow	resd	1
.lengthHigh	resd	1
.type	resd	1
.pad	resd	1	; ACPI 3.0 24byte struct
.size:
endstruc


KernelOffset	Equ	0x10000

	Jmp	0x0:Start

Start	Cld
	Cli
	Mov	ax, 0x0
	Mov	ds, ax
	Mov	es, ax
	Mov	fs, ax
	Mov	gs, ax
	Mov	ss, ax
	Mov	sp, 0x9000
	Sti

	Call	clearScreen

	Lea	si, [Greetings]
	Call	Print

	; Call	enableA20
	Call	getMemorymap

	; Call	resetFloppy
	; Call	readKernel

.this	Jmp .this

	; Cli
	; LGdt	[gdt]
	; Sti

	; Mov	eax, cr0
	; Or	eax, 1
	; Mov	cr0, eax

;	Jmp	0x08:protectedMode


; Fast enabling of the A20 should be available on all(non exotic) hardware, we'll do more if needed.
;
enableA20:	In	al, 0x92
	Test	al, 2
	Jnz	.done
	Or	al, 2
	And	al, 0xfe
	Out	0x92, al
.done	Ret

cursorY	ResW	1
cursorX	ResW	1

;
;
;
clearScreen	PushAD
	Push	es
	Mov	ax, 0xb800
	Mov	es, ax
	Mov	ax, 0x1020
	Mov	cx, 2000
	XOr	di, di
	Rep Stosw
	Mov word	[cursorY], 0
	Mov word	[cursorX], 0
	Pop	es
	PopAD
	Ret

;
;
;
Print	PushAD
	Push	es
	Mov	ax, 0xb800
	Mov	es, ax

	Mov	edi, 0

	Mov	ah, 0x21
.nextChar	Lodsb

	Cmp	al, 0xa
	Jnz	.noNewLine
	Add word	[cursorY], 160
	Mov word	[cursorX], 0
	Jmp	.nextChar

.noNewLine	Cmp	al, 0x0
	Jz	.exit

	Mov	di, [cursorY]
;	Imul	di, 160
	Add	di, [cursorX]
;	Add word	di, [cursorX]

	Mov	[es:di], ax
	Add word	[cursorX], 2
	Jmp	.nextChar

.exit	Pop	es
	PopAD
	Ret


	; Lodsb
; .nextChar	Mov	ah, 0xe
	; Mov	bx, 0x0
	; Int	0x10
	; Jc	.exit
	; Lodsb
	; Or	al, al
	; Jne	.nextChar

; .exit	Pop	es
;	Ret


readKernel	Mov	ax, KernelOffset>>4
	Mov	es, ax
	XOr	bx, bx	; Buffer offset
	Mov	ah, 2	; Read sector
	Mov	al, 20	; No. sectors
	Mov	ch, 0	; Low eight bits cylinder
	Mov	cl, 18	; Sector no. 1-63 (bits 6-7 hd only)
	Mov	dh, 0	; Head
	Mov	dl, 0	; Drive
	Int	0x13
	Jmp	KernelOffset>>4:0	


resetFloppy	Mov	ah, 0
	Mov	dl, 0
	Int	0x13
	Jc	resetFloppy
	Ret

gdtHead	Dd	0, 0		; null descriptor
 	Dw	0xFFFF, 0		; Code segment
	Db	0, 10011010b, 11001111b, 0
 	Dw	0xFFFF, 0		; Data segment
	Db	0, 10010010b, 11001111b, 0
 
gdt	Dw	gdt - gdtHead - 1
	Dd	gdtHead


buffer	resb	24	; offset(64), length(64), flags(32), pad(32)

getMemorymap	XOr	ebx, ebx
.loop	Mov	eax, 0xe820
	Mov	di, buffer
	Mov	ecx, 24
	Mov	edx, 0x0534D4150	; SMAP in little endian
	Int	0x15
	Jc	.failed
	
	Lea	si, [e820valid]
	Call	Print

.failed	ret


	; [Bits 32]

; protectedMode	Mov	ax, 0x10
	; Mov	ds, ax
	; Mov	es, ax
	; Mov	ss, ax
	; Mov	esp, 0x90000

	; Cli
	; Hlt

	[section .data]

Greetings	Db	"Loading kernel..", 0xa, 0
e820valid	Db	"E820 valid..", 0xa, 0
memoryInfo	resb	MemInfo.size

