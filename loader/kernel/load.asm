
	[bits 64]
	[section .text]

	extern ?loader@@YAXXZ
	global	_kernelstart

_kernelstart:
	mov	eax,0xdeadbeef

	jmp	?loader@@YAXXZ
