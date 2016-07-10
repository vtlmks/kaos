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


; memorylayout
;
; 0x00000 -> 0x0ffff
; ~~~~~~~~~~~~~~~~
; 0x00000 -> 0x00fff - Bios stuff
; 0x01000 -> 0x01fff - Memory list from bios 0x8e20 function (128 entries from e820, space for 170 entries if we do it in EFI)
; 0x02000 -> 0x02fff - Temporary storage for vesa-data
; 0x07c00 -> 0x07f00 - Bootblock
; 0x08000 -> 0x097ff - Loader	- Lets say it can't get larger than 6kb for the moment.
; 0x09800 -> 0x0ffff - Stack (set to 0x10000)
;
; 0x10000 - 0x1ffff	- Load the kernel here ; 448kb free, then pagetables.
; 0x20000 - 0x2ffff
; 0x30000 - 0x3ffff
; 0x40000 - 0x4ffff
; 0x50000 - 0x5ffff
; 0x60000 - 0x6ffff
; 0x70000 - 0x7ffff
; 0x80000 - 0x8ffff	- Space for temporary pagetables
; 0x90000 - 0x9fc00	- Check where the BEDA starts, pointer sholud be found at 0x040e (segment pointer, so multiply with 0x10) or something like that
; 0xa0000 - 0xbffff	- VGA display memory
; 0xc0000 - 0xc7fff	- [ROM] Video BIOS
; 0xc8000 - 0xeffff	- [ROM & RAM] Mapped hardware & misc?
; 0xf0000 - 0xfffff	- [ROM] Motherboard BIOS

KernelOffset	Equ	0x10000	; Where to load the kernel

; ==[ kernelLoader ]====================================================================[ 16bit ]==
;
	Jmp	0x0:Start

Start	Cld
	Cli
	Mov	ax, 0x0
	Mov	ds, ax
	Mov	es, ax
	Mov	fs, ax
	Mov	gs, ax
	Mov	ss, ax
	Mov	esp, 0x0000

	Call	disablePic
;	Call	disableNMI
	Sti

	; --
	Call	clearScreen

	Lea	si, [Greetings]
	Call	printString

	Call	enableA20
	Call	getMemorymap

	; load kernel code
	Call	resetFloppy
	Jmp $

	Call	loadKernel


	Cli
	LGdt	[gdt]

	Mov	eax, cr0
	Or	eax, 1
	Mov	cr0, eax


	Jmp	0x08:protectedMode


; ==[ disablePic ]======================================================================[ 16bit ]==
;
disablePic	Mov	al, 0xff
	Out	0xa1, al
	Out	0x21, al
	Ret

; ==[ disableNMI ]======================================================================[ 16bit ]==
;
disableNMI	In	al, 0x70
	And	al, 0x7f
	Out	0x70, al
	ret

; ==[ enableNMI ]=======================================================================[ 16bit ]==
;
enableNMI	In	al, 0x70
	Or	al, 0x80
	Out	0x70, al
	ret


; ==[ enableA20 ]=======================================================================[ 16bit ]==
; Fast enabling of the A20 should be available on all(non exotic) hardware, we'll do more if needed
;
; port 0x92 (System Control Port A)
;
; Bit 0 (w): writing 1 to this bit causes a fast reset (used to switch back to real mode; for MCA this took 13.4 ms). 
; Bit 1 (rw): 0: disable A20, 1: enable A20. 
; Bit 3 (rw?): 0/1: power-on password bytes (stored in CMOS bytes 0x38-0x3f or 0x36-0x3f) accessible/inaccessible. This bit can be written to only when it is 0. 
; Bits 6-7 (rw): 00: hard disk activity LED off, 01,10,11: hard disk activity LED on. 
; Bits 2,4,5 are unused or have varying meanings. (On MCA bit 4 (r): 1: watchdog timeout occurred.)
;
enableA20:	In	al, 0x92	; 0x92 (System Control Port A)
	Test	al, 2
	Jnz	.done
	Or	al, 2
	And	al, 0xfe	; Clear bit zero, just paranoia to be sure to avoid resetting the computer. TODO(peter): Should be removed! Shouldn't be possible for this to be set.
	Out	0x92, al
.done	Ret

; ==[ clearScreen ]=====================================================================[ 16bit ]==
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

; ==[ print ]===========================================================================[ 16bit ]==
;
printString	PushAD
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

; ==[ readKernel ]======================================================================[ 16bit ]==
;
loadKernel	PushAD
	Push	es
	Mov	ax, KernelOffset>>4	; This will be a temporary offset where we just load the kernel, then move it to end of memory.
	Jmp $
	Mov	es, ax
	XOr	bx, bx	; Buffer offset
	Mov	ah, 2	; Read sector
	Mov	al, 20	; No. sectors
	Mov	ch, 0	; Low eight bits cylinder
	Mov	cl, 18	; Sector no. 1-63 (bits 6-7 hd only)
	Mov	dh, 0	; Head
	Mov	dl, 0	; Drive
	Int	0x13
	Pop	es
	PopAD
	Ret

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

; ==[ getMemorymap ]====================================================================[ 16bit ]==
;
; Requests memory areas from the BIOS, adds structures to memory offset 0x1000 -> 0x1000 + 128 * 24
;
; TODO(peter): Verify that we the first requested area is duplicated always, so we can skip it!
;
getMemorymap	XOr	ebx, ebx
	Mov	eax, 0xe820	; Request memory areas
	Mov	ecx, 24	; Size of request, always request 24 bytes
	Mov	edx, "PAMS"	; SMAP in little endian
	Lea	di, [0x1000]	; Buffer to fill with data
	Int	0x15
	Jc	.failed	; Fail if BIOS-call has set the carry flag
	Cmp	eax, "PAMS"	; Fail if eax does not contain SMAP in full reverse after BIOS-call
	Jnz	.failed	;
	Test	ebx, ebx	; Fail if this is the only entry!
	Jz	.failed	;

	Lea	si, [e820valid]
	Call	printString

.nextEntry	Jcxz	.skipEntry	; If entry is zero bytes (perhaps should check if less than 20 bytes aswell)
	Lea	di, [di + 24]	; Change bufferpointer for next request

.skipEntry	Mov	eax, 0xe820
	Mov	ecx, 24	; Always request 24 bytes, even if we only get 20 back.
	Mov	edx, "PAMS"
	Int	0x15
	Jc	.done

	Test	ebx, ebx
	Jz	.done
	Jmp	.nextEntry

.done	Lea	si, [e820done]
	Call	printString
	ret

.failed	Lea	si, [e820failed]
	Call	printString
	ret

; ==[ protectedMode ]===================================================================[ 32bit ]==
;


	[Bits 32]

protectedMode	Cli
	Mov	ax, 0x10
	Mov	ds, ax
	Mov	es, ax
	Mov	ss, ax
	Mov	esp, 0x90000

	; Prepare to enter Long Mode
	Call	setupPaging

	Mov	ecx, 0xc0000080	; EFER msr
	Rdmsr
	Or	eax, 1 << 8	; LM-bit
	Wrmsr
	Mov	eax, cr0
	Or	eax, 1 << 31 | 1 << 0	; PG (Paging) + PM (Protected Mode)
	Mov	cr0, eax

	; Now we are in compatibility mode, one more thing before entering real 64bit mode

	Lgdt	[GDT64.pointer]

	; long jump to x64 from here.
	Jmp	GDT64.code:KernelOffset

	Cli
	Hlt

%define	PML4T	0x2000	; 512Gb per entry
%define	PDPT	0x3000	;   1Gb per entry
%define	PDT	0x4000	;   2Mb per entry
%define	PT	0x5000	;   4Kb per entry

setupPaging	Mov	eax, cr0	; Disable paging
	And	eax, ~(1 << 31)
	Mov	cr0, eax

	Mov	edi, 0x2000
	XOr	eax, eax
	Mov	ecx, 4096
	Rep Stosd

	Mov	edi, 0x2000
	Mov dword	[edi], 0x3003
	Add	edi, 0x1000
	Mov dword	[edi], 0x4003
	Add	edi, 0x1000
	Mov dword	[edi], 0x5003
	Add	edi, 0x1000

	Mov	ebx, 3	; R/W + Present
	Mov	ecx, 512
.setEntry	Mov	[edi], ebx
	Add	ebx, 0x1000
	Add	edi, 8
	loop	.setEntry

	Mov	edi, 0x2000	; startup paging
	Mov	cr3, edi

	Mov	eax, cr4
	Or	eax, 1 << 5	; enable PAE
	mov	cr5, eax
	Ret

	[section .data]
e820done	Db	"[BIOS] - E820 parsed..", 0xa, 0
e820failed	Db	"[BIOS] - E820 parsing failed..", 0xa, 0
e820valid	Db	"[BIOS] - E820 valid..", 0xa, 0
Greetings	Db	"Loading kernel..", 0xa, 0

GDT64:
.null	equ	$ - GDT64	; null descriptor
	Dw	0, 0
	Db	0, 0, 0, 0
.code	equ	$ - GDT64	; code descriptor
	Dw	0, 0
	Db	0, 10011010b, 00100000b, 0
.data	equ	$ - GDT64	; data descriptor
	Dw	0, 0
	Db	0, 10010010b, 00000000b, 0
.pointer	Dw	$ - GDT64 - 1	; GDT-Pointer
	Dq	GDT64

	[section .bss]
; ==[ misc screen related ]========================================================================
;
cursorY	ResW	1
cursorX	ResW	1

; ==[ e820 memory related ]========================================================================
;
e820Buffer	resb	24	; offset(64), length(64), flags(32), pad(32)
e820Data	resd	1	; Contains pointer to e820 datastorage

