	[Bits	16]
	[Section	.text]
	[Org	0x8000]

;
; Memorylayout First Megabyte
; ~~~~~~~~~~~~~~~~~~~~~~~~~~~
;
; 0x00000 -> 0x00fff	- Bios stuff
; 0x01000 -> 0x01fff	- Memory list from bios 0x8e20 function (128 entries from e820, space for 170 entries if we do it in EFI)
; 0x02000 -> 0x02fff	- Temporary storage for vesa-data
; 0x07c00 -> 0x07f00	- Bootblock
; 0x08000 -> 0x097ff	- Loader	- Lets say it can't get larger than 6kb for the moment.
; 0x09800 -> 0x0ffff	- Stack (set to 0x10000)
;
; 0x10000 -> 0x1ffff	- Load the kernel here ; 448kb free, then pagetables.
; 0x20000 -> 0x2ffff
; 0x30000 -> 0x3ffff
; 0x40000 -> 0x4ffff
; 0x50000 -> 0x5ffff
; 0x60000 -> 0x6ffff
; 0x70000 -> 0x7ffff
; 0x80000 -> 0x8ffff
; 0x90000 -> 0x9fc00	- Temporary Pagetables to get to LM from PM; Check where the EBDA starts, segment pointer should be found at 0x040e
; 0xa0000 -> 0xbffff	- VGA display memory
; 0xc0000 -> 0xc7fff	- [ROM] Video BIOS
; 0xc8000 -> 0xeffff	- [ROM & RAM] Mapped hardware & miscellaneous stuff(?)
; 0xf0000 -> 0xfffff	- [ROM] Motherboard BIOS
;
; Memory layout - Userspace
;
; 0x0000000000000000 - 0x00007fffffffffff (=47 bits) user space, different per mm
;
; Memory layout - Kernelspace
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

%include	"vesa.asm"	; vesa structures

; Miscellaneous defines
;
KernelOffset	Equ	0x10000	; Where to load the kernel

ScreenWidth	Equ	1280
ScreenHeight	Equ	720
ScreenBitsPerPixel	Equ	32

; Defines for PM paging
;
PML4T	equ	0x90000	; 512Gb per entry
PDPT	equ	0x91000	;   1Gb per entry
PDT	equ	0x92000	;   2Mb per entry
PDT2	equ	0x93000
PT	equ	0x94000	;   4Kb per entry
PT_FD000000	equ	0x95000
PT_FD200000	equ	0x96000
pgTableCount	equ	7

EFER:
.LME	equ	8

ConR4:
.PAE	equ	5

; A pointer to this structure should be supplied to the 64bit kernel in a register.
;
	struc	loaderInformation
.memInfoPtr	ResQ	1	; pointer to page where the memInfo structs are stored.
.vesaPhysBasePtr	ResQ	1	; Pointer to the LFB
.vesaBytesPerRow	ResW	1	;
.vesaPixelWidth	ResW	1	;
.vesaPixelHeight	ResW	1	;
.memInfoCount	ResB	1	; number of memory regions 1 -> 128 (e820)
.vesaPixelDepth	ResB	1	; Should never be anything but 32, but in case we want to support something else.

.size	endstruc


	struc	MemInfo
.baseAddrLow	ResD	1
.baseAddrHi	ResD	1
.lengthLow	ResD	1
.lengthHigh	ResD	1
.type	ResD	1
.pad	ResD	1	; ACPI 3.0 24byte struct
.size	endstruc


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

	Call	enableA20
	Call	enableSSE
	;
	Call	getMemorymap
	;
	Call	setVesamode
	;
	; load kernel code TODO(peter): Move the 64bit kernel to high mem after entering protected mode and paging is enabled.
	;
	Call	resetFloppy
	Call	loadKernel
	;
	; Setup and enter Protected mode, no more IRQ calls after this line, even NMI is blocked.
	;
	Cli
	Call	disableNMI
	Call	disablePic
	;
	LGdt	[GDT32.pointer]
	;
	Mov	eax, cr0
	Or	eax, 1
	Mov	cr0, eax
	;
	Jmp	GDT32.code:protectedMode
	;
enableSSE	Mov	eax, cr0
	And	ax, 0xFFFB	;clear coprocessor emulation CR0.EM
	Or	ax, 0x2	;set coprocessor monitoring  CR0.MP
	Mov	cr0, eax
	Mov	eax, cr4
	Or	ax, 3 << 9	;set CR4.OSFXSR and CR4.OSXMMEXCPT at the same time
	Mov	cr4, eax
	Ret

; ==[ playwithVesa ]====================================================================[ 16bit ]==
;
setVesamode	Mov	ax, 0x4f00
	Lea	di, [vesaInfo]
	int	0x10

; We VESA v3.0 found haz0rz

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

	Mov	ax, 0x4f01	; getVBEModeInfo
	Mov	cx, [.mode]
	Lea	di, [vesaMode]
	Int	0x10

	Cmp	ax, 0x4f
	Jne	.error

	Mov	ax, ScreenWidth
	Cmp	ax, [vesaMode + vbeMode.width]
	Jne	.nextMode

	Mov	ax, ScreenHeight
	Cmp	ax, [vesaMode + vbeMode.height]
	Jne	.nextMode

	Mov	al, ScreenBitsPerPixel
	Cmp	al, [vesaMode + vbeMode.bpp]
	Jne	.nextMode

	Bt word	[vesaMode + vbeMode.attributes], 7	; Check if it supports LFB.
	Jnc	.nextMode
	;
	; We've got a mode that we're happy about, now set it..
	;
	Mov	ax, 0x4f02
	Mov	bx, [.mode]
	Or	bx, 0x4000	; enable Linear FrameBuffer (Bit 14)
	Mov	di, 0
	Int	0x10

	Cmp	ax, 0x4f
	Jne	.error

	; Save data for LM
	;
	Mov	eax, [vesaMode + vbeMode.physicalBase]
	Mov dword	[loaderInfo + loaderInformation.vesaPhysBasePtr], eax
	Mov word	ax, [vesaMode + vbeMode.linearBytesPerRow]
	Mov word	[loaderInfo + loaderInformation.vesaBytesPerRow], ax
	Mov word	[loaderInfo + loaderInformation.vesaPixelWidth], ScreenWidth
	Mov word	[loaderInfo + loaderInformation.vesaPixelHeight], ScreenHeight
	Mov byte	[loaderInfo + loaderInformation.vesaPixelDepth], ScreenBitsPerPixel

	Clc
	Ret

.nextMode	Mov	ax, [.segment]
	Mov	fs, ax
	Mov	si, [.offset]
	Jmp	.findMode

.error	Stc
	ret

.segment	Dw	0
.offset	Dw	0
.mode	Dw	0


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
	Mov	ax, KernelOffset>>4 ; This will be a temporary offset where we just load the kernel, we'll then move it to end of memory somewhere to be forgotten..
	Mov	es, ax
	XOr	bx, bx	; Buffer offset
	Mov	ah, 2	; Read sector
	Mov	al, 20	; Number of sectors - TODO(peter): We have to fix this to the actual length
	Mov	ch, 0	; Low eight bits cylinder
	Mov	cl, 18	; Sector number 1-63 (bits 6-7 hd only)
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
getMemorymap	Mov	eax, 0xe820	; Request memory areas
	XOr	ebx, ebx
	Mov	ecx, 24	; Size of request, always request 24 bytes even if we most problably never will see that number return
	Mov	edx, "PAMS"	; SMAP in little endian
	Lea	di, [0x1000]	; Buffer to fill with data
	Int	0x15

	Jc	.failed	; BIOS calls most often sets carry on failure...

	Cmp	eax, "PAMS"	; Fail if eax does not contain PAMS after BIOS-call
	Jnz	.failed	;

	Test	ebx, ebx	; Fail if this is the only entry!
	Jz	.failed	;

	Lea	si, [e820valid]
	Call	printString

	mov byte	[loaderInfo + loaderInformation.memInfoCount], 1
	mov dword	[loaderInfo + loaderInformation.memInfoPtr], 0x1000 

.nextEntry	Jcxz	.skipEntry	; Skip if zero length (perhaps should check if less than 20 bytes as well)
	Lea	di, [di + 24]	; Change bufferpointer for next request
	Inc byte	[loaderInfo + loaderInformation.memInfoCount]

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
	Or	eax, 1 << ConR4.PAE	; enable PAE
	mov	cr4, eax
	Mov	ecx, 0xc0000080	; EFER msr
	Rdmsr
	Or	eax, 1 << EFER.LME	; LM-bit
	Wrmsr
	Mov	eax, cr0
	Or	eax, 1 << 31 | 1 << 0 ; PG (Paging) + PM (Protected Mode)
	Mov	cr0, eax

	; Now we are in compatibility mode, one more thing before entering real 64bit mode
	Lgdt	[GDT64.pointer]	; temporary 64bit GDT to get the kernel running, more of this in C layer

	Lea	edi, [loaderInfo]
	Jmp	GDT64.code:KernelOffset	; long jump to x64 from here.

	Cli	; if we were to ever get back
	Hlt	; for debugging purposes


; ==[ setupPaging ]=====================================================================[ 32bit ]==
;
setupPaging	Mov	eax, cr0	; Disable paging
	And	eax, 0x7fffffff
	Mov	cr0, eax

	;
	; HACK(peter): Clearing page memory, should be dynamic and moved to a 'set page' or something
	;

	Mov	edi, PML4T
	XOr	eax, eax
	Mov	ecx, pgTableCount * 1024
	Rep Stosd
	;

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

	; TODO(peter): Should make an initializer list for these, that we create dynamically after setting up screenmode and other stuff.

	Mov	edi, PT		; 0x00000000 -> 0x00200000 (2MB)
	Mov	ebx, 0x00000003	; R/W + Present
	Mov	ecx, 512
.setEntry	Mov	[edi], ebx
	Add	ebx, 0x1000
	Add	edi, 8
	loop	.setEntry

	Mov	edi, PT_FD000000	; 0xfd000000 -> 0xfd200000 (2MB)
	Mov	ebx, 0xfd000003
	Mov	ecx, 512
.setEntry2	Mov	[edi], ebx
	Add	ebx, 0x1000
	Add	edi, 8
	loop	.setEntry2

	Mov	edi, PT_FD200000	; 0xfd200000 -> 0xfd400000 (2MB)
	Mov	ebx, 0xfd200003
	Mov	ecx, 512
.setEntry3	Mov	[edi], ebx
	Add	ebx, 0x1000
	Add	edi, 8
	loop	.setEntry3

	Mov	edi, PML4T	; startup paging
	Mov	cr3, edi
	Ret

; =======================================[ the blitter end ]=======================================


; =================================================================================================

	[Section	.data]

GDT32:
.null	equ	$ - GDT32		; null descriptor
	Dd	0, 0
.code	equ	$ - GDT32		; Code descriptor
 	Dw	0xffff, 0
	Db	0, 10011010b, 11001111b, 0
.data	equ	$ - GDT32		; Data descriptor
 	Dw	0xffff, 0
	Db	0, 10010010b, 11001111b, 0
.pointer	Dw	$ - GDT32 - 1
	Dd	GDT32

GDT64:
.null	equ	$ - GDT64		; null descriptor
	Dd	0, 0
.code	equ	$ - GDT64		; code descriptor
	Dw	0, 0
	Db	0, 10011010b, 00100000b, 0
.data	equ	$ - GDT64		; data descriptor
	Dw	0, 0
	Db	0, 10010010b, 00000000b, 0
.pointer	Dw	$ - GDT64 - 1		; GDT-Pointer
	Dq	GDT64

; ==[ Screen related ]=============================================================================
;
cursorY	Dw	0
cursorX	Dw	0

; ==[ VESA related ]===============================================================================
;
	align 4
vesaInfo	times vbeInfo.size	db 0
	align 4
vesaMode	times vbeMode.size	db 0

; ==[ Loader related ]=============================================================================
;
	align 4
loaderInfo	times loaderInformation.size	db 0

; ==[ Strings ]====================================================================================
;
e820done	Db	"[BIOS] - E820 parsed..", 0xa, 0
e820failed	Db	"[BIOS] - E820 parsing failed..", 0xa, 0
e820valid	Db	"[BIOS] - E820 valid..", 0xa, 0

