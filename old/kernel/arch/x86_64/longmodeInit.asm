

global	longmodeStart
extern	kernelMain

	section	.text
	bits	64

longmodeStart:
	call	kernelMain
	hlt

