	[Bits	16]
	[Section	.text]
	[Org	0x8000]

%include "vesa.asm"

	struc	MemInfo
.baseAddrLow	resd	1
.baseAddrHi	resd	1
.lengthLow	resd	1
.lengthHigh	resd	1
.type	resd	1
.pad	resd	1	; ACPI 3.0 24byte struct
.size	endstruc

KernelOffset	Equ	0x10000	; Where to load the kernel


; Memorylayout
;
; 0x00000 -> 0x00fff - Bios stuff
; 0x01000 -> 0x01fff - Memory list from bios 0x8e20 function (128 entries from e820, space for 170 entries if we do it in EFI)
; 0x02000 -> 0x02fff - Temporary storage for vesa-data
; 0x07c00 -> 0x07f00 - Bootblock
; 0x08000 -> 0x097ff - Loader	- Lets say it can't get larger than 6kb for the moment.
; 0x09800 -> 0x0ffff - Stack (set to 0x10000)
;
; 0x10000 -> 0x1ffff - Load the kernel here ; 448kb free, then pagetables.
; 0x20000 -> 0x2ffff
; 0x30000 -> 0x3ffff
; 0x40000 -> 0x4ffff
; 0x50000 -> 0x5ffff
; 0x60000 -> 0x6ffff
; 0x70000 -> 0x7ffff
; 0x80000 -> 0x8ffff - Space for temporary pagetables
; 0x90000 -> 0x9fc00 - Check where the EBDA starts, pointer sholud be found at 0x040e (segment pointer, so multiply with 0x10) or something like that
; 0xa0000 -> 0xbffff - VGA display memory
; 0xc0000 -> 0xc7fff - [ROM] Video BIOS
; 0xc8000 -> 0xeffff - [ROM & RAM] Mapped hardware & misc?
; 0xf0000 -> 0xfffff - [ROM] Motherboard BIOS
;
; 0x0000000000000000 - 0x00007fffffffffff (=47 bits) user space, different per mm
;
; hole caused by [48:63] sign extension
;
; 0xffff800000000000 - 0xffffBfffffffffff (=64 TB  ) direct mapping of all phys. memory
; ... hole ...
; 0xffffc90000000000 - 0xffffe8ffffffffff (=45 bits) vmalloc/ioremap space
; ... hole ...
; 0xffffea0000000000 - 0xffffeaffffffffff (=40 bits) virtual memory map (1TB)
; ... hole ...
; 0xffffffff80000000 - 0xffffffffa0000000 (=512 MB)  kernel text mapping, from phys 0
; 0xffffffffa0000000 - 0xffffffffff5fffff (=1526 MB) module mapping space
; 0xffffffffff600000 - 0xffffffffffdfffff (=8 MB) vsyscalls
; 0xffffffffffe00000 - 0xffffffffffffffff (=2 MB) unused hole



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
	Mov	esp, 0xfff0

	Sti

	Call	setVesamode

	; --
	Call	clearScreen

	Lea	si, [Greetings]
	Call	printString

	Call	enableA20
	Call	getMemorymap

	; load kernel code
	Call	resetFloppy
	Call	loadKernel

	Cli
;	Call	disablePic
;	Call	disableNMI
	LGdt	[GDT32.pointer]

	Mov	eax, cr0
	Or	eax, 1
	Mov	cr0, eax

	Jmp	GDT32.code:protectedMode

; ==[ playwithVesa ]====================================================================[ 16bit ]==
;
setVesamode	Mov word	[.width], 1280	; TODO(peter): Change this to be configurable in a nicer way
	Mov word	[.height], 720
	Mov byte	[.bpp], 32

	Mov	ax, 0x4f00
	Mov	di, vesaInfo
	int	0x10

; we have VESA v3.0

	Mov	ax, [vesaInfo + vbeInfo.modeListPointer]
	Mov	[.offset], ax
	Mov	ax, [vesaInfo + vbeInfo.modeListPointer + 2]
	Mov	[.segment], ax
	Mov	fs, ax
	Mov	si, [.offset]

.findMode	Mov	dx, [fs:si]
	Add	si, 2
	Mov	[.offset], si
	Mov	[.mode], dx
	XOr	ax, ax
	Mov	fs, ax

	Cmp word	[.mode], 0xffff	; end of list
	Je	.error

	Mov	ax, 0x4f01	; get VBE mode info
	Mov	cx, [.mode]
	Mov	di, vesaMode
	Int	0x10

	Cmp	ax, 0x4f
	Jne	.error

	Mov	ax, [.width]
	Cmp	ax, [vesaMode + vbeMode.width]
	Jne	.nextMode

	Mov	ax, [.height]
	Cmp	ax, [vesaMode + vbeMode.height]
	Jne	.nextMode

	Mov	al, [.bpp]
	Cmp	al, [vesaMode + vbeMode.bpp]
	Jne	.nextMode

	Bt word	[vesaMode + vbeMode.attributes], 7
	Jnc	.nextMode
; we haz an screenmodez

	Mov	ax, 0x4f02
	Mov	bx, [.mode]
	Or	bx, 0x4000	; enable Linear FrameBuffer
	Mov	di, 0
	Int	0x10

	Cmp	ax, 0x4f
	Jne	.error


	Clc
	Ret

.nextMode	Mov	ax, [.segment]
	Mov	fs, ax
	Mov	si, [.offset]
	Jmp	.findMode

.error	Stc
	ret

.width	Dw	0
.height	Dw	0
.segment	Dw	0
.offset	Dw	0
.mode	Dw	0
.bpp	Db	0


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

; ==[ getMemorymap ]====================================================================[ 16bit ]==
;
; Requests memory areas from the BIOS, adds structures to memory offset 0x1000 and forwards
;
getMemorymap	XOr	ebx, ebx
	Mov	eax, 0xe820	; Request memory areas
	Mov	ecx, 24	; Size of request, always request 24 bytes
	Mov	edx, "PAMS"	; SMAP in little endian
	Lea	di, [0x1000]	; Buffer to fill with data
	Int	0x15

	Jc	.failed	; Fail if BIOS-call has set the carry flag

	Cmp	eax, "PAMS"	; Fail if eax does not contain PAMS after BIOS-call
	Jnz	.failed	;

	Test	ebx, ebx	; Fail if this is the only entry!
	Jz	.failed	;

	Lea	si, [e820valid]
	Call	printString

.nextEntry	Jcxz	.skipEntry	; Skip if zero length (perhaps should check if less than 20 bytes)
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

; =================================================================================================
	[Bits 32]

; ==[ protectedMode ]===================================================================[ 32bit ]==
;
protectedMode	Cli
	Mov	ax, 0x10
	Mov	ds, ax
	Mov	es, ax
	Mov	ss, ax
	Mov	esp, 0x90000

	; Prepare to enter Long Mode
	Call	setupPaging

	Mov	eax, cr4
	Or	eax, 1 << 5	; enable PAE
	mov	cr4, eax
	Mov	ecx, 0xc0000080	; EFER msr
	Rdmsr
	Or	eax, 1 << 8	; LM-bit
	Wrmsr
	Mov	eax, cr0
	Or	eax, 1 << 31 | 1 << 0 ; PG (Paging) + PM (Protected Mode)
	Mov	cr0, eax

	; Now we are in compatibility mode, one more thing before entering real 64bit mode
	Lgdt	[GDT64.pointer]	; temporary 64bit GDT to get the kernel running, more of this in C layer

	Jmp	GDT64.code:KernelOffset	; long jump to x64 from here.

	Cli	; if we were to ever get back
	Hlt	; for debugging purposes


; ==[ setupPaging ]=====================================================================[ 32bit ]==
;
%define	PML4T	0x2000	; 512Gb per entry
%define	PDPT	0x3000	;   1Gb per entry
%define	PDT	0x4000	;   2Mb per entry
%define	PDT2	0x12000	;   2Mb per entry
%define	PT	0x5000	;   4Kb per entry
%define	PT_FD000000	0x6000
%define	PT_FD200000	0x13000

setupPaging	Mov	eax, cr0	; Disable paging
	And	eax, 0x7fffffff
	Mov	cr0, eax

	Mov	edi, 0x2000
	XOr	eax, eax
	Mov	ecx, 6 * 1024
	Rep Stosd

	Mov	edi, 0x12000
	XOr	eax, eax
	Mov	ecx, 6 * 1024
	Rep Stosd


; fd000000

; 11 111101000 000000000 000000000000
; PML4= 0
; pdpt= 488th
; pdt = 0th
; pt  = 512 pages @ 0x7000


	Mov	edi, PML4T
	Mov dword	[edi], PDPT | 3

	Mov	edi, PDPT
	Mov dword	[edi], PDT | 3
	Mov dword	[edi + 3 * 8], PDT2 | 3

	Mov	edi, PDT
	Mov dword	[edi], PT | 3

	Mov	edi, PDT2
	Mov dword	[edi + 488 * 8], PT_FD000000 | 3
	Mov dword	[edi + 489 * 8], PT_FD200000 | 3

	Mov	edi, PT
	Mov	ebx, 3	; R/W + Present
	Mov	ecx, 512
.setEntry	Mov	[edi], ebx
	Add	ebx, 0x1000
	Add	edi, 8
	loop	.setEntry

	Mov	edi, PT_FD000000
	Mov	ebx, 0xfd000003	; R/W + Present
	Mov	ecx, 512
.setEntry2	Mov	[edi], ebx
	Add	ebx, 0x1000
	Add	edi, 8
	loop	.setEntry2

	Mov	edi, PT_FD200000
	Mov	ebx, 0xfd200003	; R/W + Present
	Mov	ecx, 512
.setEntry3	Mov	[edi], ebx
	Add	ebx, 0x1000
	Add	edi, 8
	loop	.setEntry3

	Mov	edi, 0x2000	; startup paging
	Mov	cr3, edi
	Ret

; =======================================[ the blitter end ]=======================================

	[Section	.data]
; =================================================================================================

e820done	Db	"[BIOS] - E820 parsed..", 0xa, 0
e820failed	Db	"[BIOS] - E820 parsing failed..", 0xa, 0
e820valid	Db	"[BIOS] - E820 valid..", 0xa, 0
Greetings	Db	"Loading kernel..", 0xa, 0

GDT32:
.null	equ	$ - GDT32	; null descriptor
	Dd	0, 0
.code	equ	$ - GDT32	; Code descriptor
 	Dw	0xffff, 0
	Db	0, 10011010b, 11001111b, 0
.data	equ	$ - GDT32	; Data descriptor
 	Dw	0xffff, 0
	Db	0, 10010010b, 11001111b, 0
.pointer	Dw	$ - GDT32 - 1
	Dd	GDT32


GDT64:
.null	equ	$ - GDT64	; null descriptor
	Dd	0, 0
.code	equ	$ - GDT64	; code descriptor
	Dw	0, 0
	Db	0, 10011010b, 00100000b, 0
.data	equ	$ - GDT64	; data descriptor
	Dw	0, 0
	Db	0, 10010010b, 00000000b, 0
.pointer	Dw	$ - GDT64 - 1	; GDT-Pointer
	Dq	GDT64

	[Section	.bss]
; ==[ misc screen related ]========================================================================
;
cursorY	ResW	1
cursorX	ResW	1

; ==[ e820 memory related ]========================================================================
;
e820Data	ResD	1	; Contains pointer to e820 datastorage

; ==[ VESA related ]===============================================================================
;
vesaInfo	ResB	vbeInfo.size
vesaMode	ResB	vbeMode.size
