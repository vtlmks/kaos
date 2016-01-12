; ex:set ts=12
; TODO(peter): use macros for these flags/bitsets

MBALIGN     equ	1<<0		; align loaded modules on page boundaries
MEMINFO     equ	1<<1		; provide memory map
VIDMODE	equ	1<<2
FLAGS       equ	MBALIGN | MEMINFO | VIDMODE	; this is the Multiboot 'flag' field
MAGIC       equ	0x1BADB002		; 'magic number' lets bootloader find the header
CHECKSUM    equ	-(MAGIC + FLAGS)	; checksum of above, to prove we are multiboot

	section .multiboot
	align	4
	dd	MAGIC		; magic
	dd	FLAGS		; flags
	dd	CHECKSUM		; -(MAGIC + FLAGS)
	dd	0		; load addr     (if flag bit 16 set)
	dd	0		; load end addr (if flag bit 16 set)
	dd	0		; header_addr   (if flag bit 16 set)
	dd	0		; bss end addr  (if flag bit 16 set)
	dd	0		; entry addr    (if flag bit 16 set)
	dd	1		; 1-textmode ; 0-linear graphics mode
	dd	1280		; width
	dd	720		; height
	dd	8		; depth

	section .bootstrap_stack, nobits
	align	4
stack_bottom:
	resb	16384
stack_top:

; When we get to _start we are in 32-bit protected mode, 
;
; eax has to be 0x2badb002
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
;
; Multiboot information structure
;
;	dd flags		(required)
;	dd mem_lower	(present if flags[0] is set)
;	dd mem_upper
;	dd boot_device	(present if flags[1] is set)
;	dd cmdline
;	dd mods_count	(present if flags[3] is set)
;	dd mods_addr
;	dd syms		(present if flags[4] or flags[5] is set)
;	dd syms
;	dd syms
;	dd syms
;	dd mmap_length	(present if flags[6] is set)
;	dd mmap_addr
;	dd drives_length	(present if flags[7] is set)
;	dd drives_addr
;	dd config_table	(present if flags[8] is set)
;	dd boot_loader_name	(present if flags[9] is set)
;	dd apm_table	(present if flags[10] is set)
;	dd vbe_control_info	(present if flags[11] is set)
;	dd vbe_mode_info
;	dd vbe_mode
;	dd vbe_interface_seg
;	dd vbe_interface_off
;	dd vbe_interface_len

	global _start
	extern	kernel_main

	section	.text
	bits	32
_start:	mov	esp, stack_top
	push eax
	push ebx
	call	kernel_main
	cli
.hang:	hlt
	jmp	.hang

