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
; 0x0000000000100000 - 0x00007fffffffffff (=47 bits) user space, different per mm
;
; Memory layout - Kernelspace - Hole caused by [48:63] sign extension
;
; 0xfff0000000000000 - 0xffffbfffffffffff (=64 TB  ) direct mapping of all phys. memory
; ... Hole ...
; 0xffffffff80000000 - 0xffffffffa0000000 (=512 MB)  kernel text mapping, from phys 0
; 0xffffffffa0000000 - 0xffffffffff5fffff (=1526 MB) module mapping space

%include	"vesa.asm"	; vesa structures

; Miscellaneous defines
;
KernelOffset	Equ	0x10000	; Where to load the kernel

;
; screen mode we will be looking for, we know this exist on qemu and bochs,
; and we've added it to the virtualbox configuration that we test with
;
ScreenWidth	Equ	1280
ScreenHeight	Equ	720
ScreenBitsPerPixel	Equ	32

;
MSR_EFER	Equ	0xc0000080

; Defines for Memoryprobe
;
e820Buffer	Equ	0x1000
PGTable	Equ	0x90000

EFER:
.LME	Equ	8

CR_4:
.PAE	Equ	5
.OSFXSR	Equ	9
.OSXMMEXCPT	Equ	10

CR_0:
.PE	Equ	0
.MP	Equ	1
.EM	Equ	2
.PG	Equ	31

PT:
.P	Equ	0	; Present
.RW	Equ	1	; Read/Write
.US	Equ	2	; User/Supervisor
.PWT	Equ	3	; Write-Through
.PCD	Equ	4	; Cache Disabled
.A	Equ	5	; Accessed
.D	Equ	6	; Dirty
.PAT	Equ	7	; Page Attribute Table Index - This is combined with bit 3 and 4 to point at different PAT's
.G	Equ	8	; Global page

PD:
.P	Equ	0	; Present
.RW	Equ	1	; Read/Write
.US	Equ	2	; User/Supervisor
.PWT	Equ	3	; Write-Through
.PCD	Equ	4	; Cache Disabled
.A	Equ	5	; Accessed
;		6	; Reserved
.PS	Equ	7	; Page size (0 indicates 4KBytes)
.G	Equ	8	; Global page (ignored)

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
.size	endstruc

	[Bits 16]
	[Org 0x8000]
	[Section .text]

; ==[ kernelLoader ]====================================================================[ 16bit ]==
;
	Jmp	0x0:Start	; make sure CS is 0x0

Start	Cld
	Cli
	Mov	ax, 0x0
	Mov	ds, ax
	Mov	es, ax
	Mov	fs, ax
	Mov	gs, ax
	Mov	ss, ax
	Mov	sp, 0xfff0
	Sti

	Call	enableA20
	Call	enableSSE
	Call	getMemorymap
	Call	setVesamode
	;
	; load kernel code TODO(peter): Move the 64bit kernel to high mem after entering protected mode and paging is enabled.
	;
	Call	loadKernel
	;
	Cli
	Call	disableNMI
	Call	disablePic
	;
	; Setup and enter Protected mode, no more IRQ calls after this line, even NMI is blocked.
	;
	LGdt	[GDT32.pointer]
	;
	Mov	eax, cr0
	Or	eax, 1 << CR_0.PE
	Mov	cr0, eax
	;
	Jmp	GDT32.code:protectedMode

; ==[ enableSSE ]=======================================================================[ 16bit ]==
enableSSE	Mov	eax, cr0
	And	ax, 0xfffb	;clear coprocessor emulation CR0.EM
	Or	ax, 1 << CR_0.MP	;set coprocessor monitoring  CR0.MP
	Mov	cr0, eax
	Mov	eax, cr4
	Or	ax, 1 << CR_4.OSFXSR | 1 << CR_4.OSXMMEXCPT	;set CR4.OSFXSR and CR4.OSXMMEXCPT at the same time
	Mov	cr4, eax
	Ret

; ==[ setVesaMode ]=====================================================================[ 16bit ]==
setVesamode	Mov	ax, 0x4f00
	Lea	di, [vesaInfo]
	int	0x10

	Mov	ax, [vesaInfo + vbeInfo.modeListPtr + farPtr.offset]
	Mov	[.offset], ax
	Mov	ax, [vesaInfo + vbeInfo.modeListPtr + farPtr.segment]
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
	Cmp	ax, [vesaMode + vbeModeInfo.screenWidth]
	Jne	.nextMode

	Mov	ax, ScreenHeight
	Cmp	ax, [vesaMode + vbeModeInfo.screenHeight]
	Jne	.nextMode

	Mov	al, ScreenBitsPerPixel
	Cmp	al, [vesaMode + vbeModeInfo.bpp]
	Jne	.nextMode

	Bt word	[vesaMode + vbeModeInfo.attributes], 7	; Check if it supports LFB.
	Jnc	.nextMode

	; We've got a mode that we're happy about, now set it..

	Mov	ax, 0x4f02
	Mov	bx, [.mode]
	Or	bx, 0x4000	; enable Linear FrameBuffer (Bit 14)
	Mov	di, 0
	Int	0x10

	Cmp	ax, 0x4f
	Jne	.error

	; Save data for the Kernel
	;
	Mov	eax, [vesaMode + vbeModeInfo.lfbPtr]
	Mov dword	[loaderInfo + loaderInformation.vesaPhysBasePtr], eax
	Mov word	ax, [vesaMode + vbeModeInfo.linearBytesPerRow]
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

	[Section .bss]
.segment	resw	1
.offset	resw	1
.mode	resw	1

	[Section .text]
; ==[ disablePic ]======================================================================[ 16bit ]==
disablePic	Mov	al, 0xff
	Out	0xa1, al
	Out	0x21, al
	Ret

; ==[ disableNMI ]======================================================================[ 16bit ]==
disableNMI	In	al, 0x70
	And	al, 0x7f
	Out	0x70, al
	ret

; ==[ enableNMI ]=======================================================================[ 16bit ]==
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

; ==[ loadKernel ]======================================================================[ 16bit ]==
loadKernel	PushAD
	Push	es
	Call	resetFloppy

	Mov	ax, KernelOffset>>4	; This will be a temporary offset where we just load the kernel, we'll then move it to end of memory somewhere to be forgotten..
	Mov	es, ax
	XOr	bx, bx	; Buffer offset
	Mov	ah, 2	; Read sector
	Mov	al, 50	; Number of sectors - TODO(peter): We have to fix this; obtain the actual length
	Mov	ch, 0	; Low eight bits cylinder
	Mov	cl, 18	; Sector number 1-63 (bits 6-7 hd only)
	Mov	dh, 0	; Head
	Mov	dl, 0	; Drive
	Int	0x13
	Pop	es
	PopAD
	Ret

; ==[ resetFloppy ]=====================================================================[ 16bit ]==
resetFloppy	Mov	ah, 0
	Mov	dl, 0
	Int	0x13
	Jc	resetFloppy
	Ret

; ==[ getMemorymap ]====================================================================[ 16bit ]==
; Requests memory areas from the BIOS, adds structures to memory offset 0x1000 and forwards
;
getMemorymap	Mov	eax, 0xe820	; Request memory areas
	XOr	ebx, ebx
	Mov	ecx, 20
	Mov	edx, "PAMS"	; SMAP in little endian
	Lea	di, [e820Buffer]	; Buffer to fill with data
	Int	0x15

	Jc	.failed	; BIOS calls most often sets carry on failure...

	Cmp	eax, "PAMS"	; Fail if eax does not contain PAMS after BIOS-call
	Jnz	.failed	;

	Test	ebx, ebx	; Fail if this is the only entry!
	Jz	.failed	;

;	Lea	si, [e820valid]
;	Call	printString

	mov byte	[loaderInfo + loaderInformation.memInfoCount], 1
	mov dword	[loaderInfo + loaderInformation.memInfoPtr], e820Buffer

.nextEntry	Jcxz	.skipEntry	; Skip if zero length (perhaps should check if less than 20 bytes as well)
	Lea	di, [di + 20]	; Change bufferpointer for next request
	Inc byte	[loaderInfo + loaderInformation.memInfoCount]

.skipEntry	Mov	eax, 0xe820
	Mov	ecx, 20
	Mov	edx, "PAMS"
	Int	0x15
	Jc	.done

	Test	ebx, ebx
	Jz	.done
	Jmp	.nextEntry

.done	ret
	;Lea	si, [e820done]
	;Call	printString

.failed	ret
;	Lea	si, [e820failed]
;	Call	printString

; =================================================================================================
	[Bits 32]

; ==[ protectedMode ]===================================================================[ 32bit ]==
protectedMode	Cli
	Mov	ax, 0x10
	Mov	ds, ax
	Mov	es, ax
	Mov	fs, ax
	Mov	gs, ax
	Mov	ss, ax
	Mov	esp, 0x90000
	;
	; Prepare to enter Long Mode
	;
	; Technically we don't need to enter protected mode before
	; entering LongMode but some things are easier to do in
	; protected mode, more things will be added here in the future.
	;
	Mov	eax, cr4
	Or	eax, 1 << CR_4.PAE
	mov	cr4, eax
	;
	Call	setupPaging
	;
	Mov	ecx, MSR_EFER	; EFER msr
	Rdmsr
	Or	eax, 1 << EFER.LME	; Long Mode Enabled
	Wrmsr
	;
	Mov	eax, cr0
	Or	eax, 1 << CR_0.PG	; Paging Enabled
	Mov	cr0, eax
	;
	; Now we are in compatibility mode
	;
	Lgdt	[GDT64.pointer]	; temporary 64bit GDT to get the kernel running, more of this in C layer
	;
	Lea	edi, [loaderInfo]	; Information that we want to send to the 64-bit side.
	Jmp	GDT64.code:KernelOffset	; long jump to x64 from here.
	;
	Cli	; if we were to ever get back
	Hlt	; for debugging purposes

; ==[ setupPaging ]=====================================================================[ 32bit ]==
;
; Early paging setup, here we use 2MB pages to page the entire 32bit area...
;
setupPaging	Mov	eax, cr0	; Disable paging
	And	eax, 0x7fffffff
	Mov	cr0, eax

	Lea	edi, [PGTable]	; Clear 6 pagetable entries (24kb)
	Xor	eax, eax
	Mov	ecx, (4096*6)/4
	Rep stosd

	Lea	edi, [PGTable]	; Setup the first PML4 entry to point to first PD
	Lea	eax, [edi + 0x1007]
	Mov	[edi], eax

	Lea	edi, [PGTable + 0x1000] ; Setup 4 pagetable entries
	Lea	eax, [edi + 0x1007]
	Mov	ecx, 4
.l	Mov	[edi], eax
	Add	eax, 0x1000
	Add	edi, 8
	Dec	ecx
	Jnz	.l

	Lea	edi, [PGTable + 0x2000] ; Setup 2MB pages for the entire 4GB area
	Mov	eax, 0x183
	Mov	ecx, 2048
.l2	Mov	[edi], eax
	Add	eax, 0x200000
	Add	edi, 8
	Dec	ecx
	Jnz	.l2

	Lea	eax, [PGTable]
	Mov	cr3, eax
	Ret

; =================================================================================================
; =======================================[ the blitter end ]=======================================
; =================================================================================================
	[Section .data]
GDT32:
.null	equ	$ - GDT32		; null descriptor
	Dq	0
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
	Dq	0
.code	equ	$ - GDT64		; code descriptor
	Dd	0
	Db	0, 10011010b, 00100000b, 0
.data	equ	$ - GDT64		; data descriptor
	Dd	0
	Db	0, 10010010b, 00000000b, 0
.pointer	Dw	$ - GDT64 - 1		; GDT-Pointer
	Dq	GDT64

	[Section .bss]
; ==[ VESA related ]===============================================================================
vesaInfo	resb	vbeInfo.size
vesaMode	resb	vbeModeInfo.size

; ==[ Loader related ]=============================================================================
loaderInfo	resb	loaderInformation.size

; ==[ Screen related ]=============================================================================
cursorY	resw	1
cursorX	resw	1

; OLD cruft, might be usable for debugging, but ... if so, enable

;	[section .text]
;
; ==[ clearScreen ]=====================================================================[ 16bit ]==
;clearScreen	PushAD
;	Push	es
;	Mov	ax, 0xb800
;	Mov	es, ax
;	Mov	ax, 0x1020
;	Mov	cx, 2000
;	XOr	di, di
;	Rep Stosw
;	Mov word	[cursorY], 0
;	Mov word	[cursorX], 0
;	Pop	es
;	PopAD
;	Ret

; ==[ printString ]=====================================================================[ 16bit ]==
;printString	PushAD
;	Push	es
;
;	Mov	ax, 0xb800
;	Mov	es, ax
;
;	Mov	ah, 0x13
;.nextChar	Lodsb
;
;	Cmp	al, 0xa
;	Jnz	.noNewLine
;	Add word	[cursorY], 160
;	Mov word	[cursorX], 0
;	Jmp	.nextChar
;
;.noNewLine	Cmp	al, 0x0
;	Jz	.exit
;
;	Mov	di, [cursorY]
;	Add	di, [cursorX]
;
;	Mov	[es:di], ax
;	Add word	[cursorX], 2
;	Jmp	.nextChar
;
;.exit	Pop	es
;	PopAD
;	Ret

