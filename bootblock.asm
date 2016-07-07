
	Bits	16
	Section	.text
	Org	0x7c00

Stage2Offset	Equ	0x8000

start: 	jmp	start0

    	Db	'KAOS 0.0'
    	Dw	0x200	; bytes/sector
    	Db	0x1	; sector/cluster
    	Dw	0x1	; sector reserved
    	Db	0x2	; FATs
    	Dw	0xe0	; root directory entries
    	Dw	0xb40	; sectors
    	Db	0xf0	; media
    	Dw	0x9	; sectors/FAT
    	Dw	0x12	; sectors/track
    	Dw	0x2	; heads
    	Dd	0	; hidden sectors
    	Dd	0	; sectors again
    	Db	0	; drive
	Db	0	; reserved
	Db	0x29	; signature
	Dd	0xa0a1a2a3	; volumeid
	Db	"KAOS FLOPPY"	; volumename
	Db	"FAT12   "	; filesystem type

Greetings	Db	"Loading...", 0

Print	Lodsb
.nextChar	Mov	bx, 0x1
	Mov	ah, 0xe
	Int	0x10
	Lodsb
	Test	al, al
	Jne	.nextChar
	Ret

start0:	Cld
	Cli
	XOr	ax, ax
	Mov	ds, ax
	Mov	es, ax
	Sti

	mov	si, Greetings
	Call	Print

.Reset:	XOr	ah, ah
	XOr	dh, dh
	Int	0x13
	Jc	.Reset

	Mov	ax, Stage2Offset>>4
	Mov	es, ax
	XOr	bx, bx	; Buffer offset
	Mov	ah, 2	; Read sector
	Mov	al, 16	; No. sectors
	Mov	ch, 0	; Low eight bits cylinder
	Mov	cl, 2	; Sector no. 1-63 (bits 6-7 hd only)
	Mov	dh, 0	; Head
	Mov	dl, 0	; Drive
	Int	0x13

	Jmp	Stage2Offset>>4:0

	Times 510-($-$$)	Db 0
	Dw	0xaa55	; Magic
