; vim:set tabstop=14:

	section .multiboot_header

	align 8
headerStart:
	dd	0xe85250d6					; magic number (multiboot 2)
	dd	0								; architecture 0 (protected mode i386)
	dd	headerEnd - headerStart	; header length
	dd	0x100000000 - (0xe85250d6 + 0 + (headerEnd - headerStart))	; checksum

	; insert optional multiboot tags here

	; align 8

	; dw	5	; framebufferTag
	; dw	0
	; dd	20
	; dd	1280
	; dd	720
	; dd	8

	align 8

	dw	0	; endTag
	dw	0
	dd	8
headerEnd:
