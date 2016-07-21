#include "include\assembler.h"
#include "include\types.h"


struct IDTDescriptor {
   u16 	offsetLow;		// bits 0..15
   u16 	selector; 		// code segment
   u8	reserved;
   u8	typeAttributes; // type and attributes
   u16	offsetMiddle; 	// bits 16..31
   u32	offsetHigh;		// bits 32..63
};

struct __attribute__ ((__packed__)) LIDTEntry {
	u16			Limit;
	u64			BaseAddress;
};

LIDTEntry		IDT;
IDTDescriptor	IDTTable[256];	
u64 			dummy;
u64 			dummy2;

void interruptTest() {

	asm("mov $0xdeadb00f,%eax");
	asm("jmp .;");
}

void interruptsInit() {
	IDT.Limit = sizeof(IDTDescriptor)*256-1;
	IDT.BaseAddress = (u64)&IDTTable;

	for(int i=0;i<256;i++)
	{
		IDTTable[i].offsetLow = (u16) ((u64)(&interruptTest)&0xffff);
		IDTTable[i].selector = 8;
		IDTTable[i].typeAttributes = 0x80 | 0xe;		// present-bit, interrupt gate
		IDTTable[i].offsetMiddle = (u16) ((u64)(&interruptTest)>>16);
		IDTTable[i].offsetHigh = (u32) ((u64)(&interruptTest)>>32);
	}
	asm("lidt %0" : : "m" (IDT));
	asm("sti;");
}
