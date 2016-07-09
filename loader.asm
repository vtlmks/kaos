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
.size	endstruc


KernelOffset	Equ	0x10000	; Where to load the kernel

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
	Call	print

	Call	enableA20
	Call	getMemorymap

	Call	resetFloppy
	Call	readKernel

	Cli
	LGdt	[gdt]
	Sti

	Mov	eax, cr0
	Or	eax, 1
	Mov	cr0, eax

	Jmp	0x08:protectedMode


; ==[ enableA20 ]==================================================================================
; Fast enabling of the A20 should be available on all(non exotic) hardware, we'll do more if needed
;
enableA20:	In	al, 0x92
	Test	al, 2
	Jnz	.done
	Or	al, 2
	And	al, 0xfe
	Out	0x92, al
.done	Ret

; ==[ clearScreen ]================================================================================
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

; ==[ print ]======================================================================================
;
print	PushAD
	Push	es
	Mov	ax, 0xb800
	Mov	es, ax

	Mov	ah, 0x13
.nextChar	Lodsb

	Cmp	al, 0xa
	Jnz	.noNewLine
	Add word	[cursorY], 160
	Mov word	[cursorX], 0
	Jmp	.nextChar

.noNewLine	Cmp	al, 0x0
	Jz	.exit

	Mov	di, [cursorY]
	Add	di, [cursorX]

	Mov	[es:di], ax
	Add word	[cursorX], 2
	Jmp	.nextChar

.exit	Pop	es
	PopAD
	Ret

; ==[ readKernel ]=================================================================================
;
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

; ==[ getMemorymap ]===============================================================================
;
getMemorymap	XOr	ebx, ebx
	Mov	eax, 0xe820
	Mov	ecx, 20
	Mov	edx, "PAMS"	; SMAP in little endian
	Lea	di, [0x1000]
	Int	0x15
	Jc	.failed
	cmp	eax, "PAMS"	; SMAP in full reverse
	Jnz	.failed
	XOr	ebx, ebx	; is this the only entry?
	Jne	.failed

	Lea	si, [e820valid]
	Call	print

.nextEntry	Jcxz	.skipEntry	; if entry is zero bytes (perhaps check if less than 20 bytes aswell)
	Lea	di, [di + 0x20]

.skipEntry	Mov	eax, 0xe820
	Mov	ecx, 20
	Mov	edx, "PAMS"
	Int	0x15
	Jc	.done

	Test	ebx, ebx
	Jz	.done
	Jmp	.nextEntry

.done	Lea	si, [e820done]
	Call	print
	ret

.failed	Lea	si, [e820failed]
	Call	print
	ret


	[Bits 32]
; ==[ protectedMode ]==============================================================================
;
protectedMode	Cli
	Mov	ax, 0x10
	Mov	ds, ax
	Mov	es, ax
	Mov	ss, ax
	Mov	esp, 0x90000
	Sti

	Cli
	Hlt

	[section .data]
e820done	Db	"[BIOS] - E820 parsed..", 0xa, 0
e820failed	Db	"[BIOS] - E820 parsing failed..", 0xa, 0
e820valid	Db	"[BIOS] - E820 valid..", 0xa, 0
Greetings	Db	"Loading kernel..", 0xa, 0

	[section .bss]
; ==[ misc screen related ]========================================================================
;
cursorY	ResW	1
cursorX	ResW	1

; ==[ e820 memory related ]========================================================================
;
e820Buffer	resb	24	; offset(64), length(64), flags(32), pad(32)
e820Data	resd	1	; Contains pointer to e820 datastorage

