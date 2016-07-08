
	[bits 16]
	[section .text]

	org	0x8000

loaderEntry:	cli
	jmp	0x8:.gdt
.gdt	mov	ax, 0x10
	mov	ds, ax
	mov	es, ax
	mov	fs, ax
	mov	gs, ax
	mov	ss, ax

	sti

	call	enableA20
	ret


; Fast enabling of the A20 should be available on all(non exotic) hardware since PS/2
;
enableA20:	in	al, 0x92
	test	al, 2
	jnz	.done
	or	al, 2
	and	al, 0xfe
	out	0x92, al
.done	ret
