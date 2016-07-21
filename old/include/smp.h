


check BSP flag in the IA32_APIC_BASE MSR

bit 8 - indicates if the processor is the BSP
bit 11 - apic global enable flag.
bits 12->35 APIC base field <<12 to form a 36 bit address on a 4k boundary.

The MAXPHYADDR is 36 bits for processors that do not support CPUID leaf 80000008H,
or indicated by CPUID.80000008H:EAX[bits 7:0] for processors that support CPUID leaf 80000008H

MAXPHYADDR = 36
bits 0->7, 9, 10 and MAXPHYADDR->63 are reseved.



ICR_LOW		EQU 0FEE00300H
SVR		EQU 0FEE000F0H
APIC_ID		EQU 0FEE00020H
LVT3		EQU 0FEE00370H
APIC_ENABLED	EQU 0100H
BOOT_ID		DD ?
COUNT		EQU 00H
VACANT		EQU 00H

	mov esi, APIC_ID
	mov	eax, [esi]
	and	eax, 0xff000000
	mov	BOOT_ID, eax

the APIC default base memory addresses defined by the MP specification are
0xFEC00000 I/O APIC ; If there are more than one, the next is offset by 0x1000
0xFEE00000 LOCAL APIC

from MP Specification 1.4:
The hardware must support a mode of operation in which the system can switch easily to
Symmetric I/O mode from PIC or Virtual Wire mode. When the operating system is ready to
switch to MP operation, it writes a 01H to the IMCR register, if that register is
implemented, and enables I/O APIC Redirection Table entries. The hardware must not
require any other action on the part of software to make the transition to Symmetric I/O mode.

Before we are in symmetric I/O Mode we get FP-errors to IRQ13,
after we've entered Symmetric I/O we get them on IRQ 16

MP Floating Pointer Structure
a) first KB of extended BIOS area (EBDA), or
b) WIthin the last kilobyte of system base memory (639->640k), or
c) in the BIOS ROM address space between 0xf0000 -> 0xfffff

start address for EBDA is found at $40:0e of the BIOS area


test:
~~~~~
_MP_ on qemu

000F:6B80 | 5F 4D 50 5F  90 6B 0F 00  01 04 96 00  00 00 00 00 | _MP_.k..........

struct MP {
	_MP_
	000f6b90
	00960401 -> points to MP Configuration Table Header
	00000000
}

000F:6B90 | 50 43 4D 50  D0 00 04 2A  42 4F 43 48  53 43 50 55 | PCMPÐ..*BOCHSCPU
000F:6BA0 | 30 2E 31 20  20 20 20 20  20 20 20 20  00 00 00 00 | 0.1         ....
000F:6BB0 | 00 00 13 00  00 00 E0 FE  00 00 00 00

struct PCMP {
	PCMP
	2a0400d0	BASE Table Length = 0xd0, SPEC_REV = 0x4, CHECKSUM = 0x2a
	"BOCH"		OEM_ID[64]
	"SCPU"
	"0.0 "		PRODUCT_ID[96]
	"    "
	"    "
	00000000	OEM Table Pointer
	00130000	Entry Count = 0x13; OEM TABLE SIZE = 0
	fee00000	- Local APIC
	00000000
}





000F:6B90 | 50 43 4D 50  D0 00 04 2A  42 4F 43 48  53 43 50 55 | PCMPÐ..*BOCHSCPU
000F:6BA0 | 30 2E 31 20  20 20 20 20  20 20 20 20  00 00 00 00 | 0.1         ....
000F:6BB0 | 00 00 13 00  00 00 E0 FE  00 00 00 00  00 00 14 03 | ......àþ........
000F:6BC0 | 63 06 00 00  FD FB 8B 07  00 00 00 00  00 00 00 00 | c...ýû..........
000F:6BD0 | 01 00 50 43  49 20 20 20  01 01 49 53  41 20 20 20 | ..PCI   ..ISA   
000F:6BE0 | 02 00 11 01  00 00 C0 FE  03 00 01 00  00 08 00 0B | ......Àþ........
000F:6BF0 | 03 00 01 00  00 7C 00 0A  03 00 00 00  01 00 00 02 | .....|..........
000F:6C00 | 03 00 00 00  01 01 00 01  03 00 00 00  01 03 00 03 | ................
000F:6C10 | 03 00 00 00  01 04 00 04  03 00 00 00  01 06 00 06 | ................
000F:6C20 | 03 00 00 00  01 07 00 07  03 00 00 00  01 08 00 08 | ................
000F:6C30 | 03 00 00 00  01 0C 00 0C  03 00 00 00  01 0D 00 0D | ................
000F:6C40 | 03 00 00 00  01 0E 00 0E  03 00 00 00  01 0F 00 0F | ................
000F:6C50 | 04 03 00 00  01 00 00 00  04 01 00 00  01 00 FF 01 | ..............ÿ.

