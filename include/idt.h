
64bit mode stack frame: 6.14.2

in 64bit mode, the size of interrupt stack-frame pushes it fixed at eight bytes.
64bit mode also pushes SS:RSP unconditionally, rather than only on a CPL change.
Aside from error codes, pushing SS:RSP unconditionally presents operating systems
with a consistent interrupt-stackframe size across all interrupts.

interrupt service-routine entry points that handle interrupts generated by the INTn instruction or
external INTR# signal can push an additional error code place-holder to maintain consistency.

The RSP is aligned to a 16byte boundary before pushing the stackframe;
the stackframe itself is aligned on a 16byte boundary when the interrupt handler is
called. The processor can arbitrarily realign the new RSP on interrupts because the
previous (possibly unaligned) RSP is unconditionally saved on the newly aligned stack.
The previous RSP will be automatically restored by a subsequent IRET



struct IdtEntry {
	u16	offsetLow;	// bit 0->15
	u16	selector;	// Code segment selector
	u08	ist;		// Interrupt stack table
	u08	flags;		// Type and Attributes; present, descriptor privilege level
	u16	offsetMid;	// bit 16->31
	u32	offsetHigh;	// bit 32->63
	u32	reserved;	// zero
};

typefield: (ia-32e mode only (64bit)).
bits 11-8

 0010 - LDT
 1001 - 64bit tss (available)
 1011 - 64bit tss (busy)
 1100 - 64bit call gate
 1110 - 64bit interrupt gate
 1111 - 64bit trap gate

