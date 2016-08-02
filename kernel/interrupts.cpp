#include "include\assembler.h"
#include "include\types.h"


struct IDTDescriptor {
   u16 	offsetLow;		// bits 0..15
   u16 	selector; 		// code segment
   u8	reserved;
   u8	typeAttributes; // type and attributes
   u16	offsetMiddle; 	// bits 16..31
   u32	offsetHigh;		// bits 32..63
   u32	reserved2;
};

struct __attribute__ ((__packed__)) LIDTEntry {
	u16			Limit;
	u64			BaseAddress;
};

LIDTEntry		IDT;
IDTDescriptor	IDTTable[256];	
u64 			dummy;
u64 			dummy2;


int kprintf(const char *formatString, ...);

void interruptTest() {

	asm("mov $0xdeadffff,%eax;");
	asm("jmp .;");
}

void irq0() {
	kprintf("mokaw\n");
	u32* v = (u32*)0xfd000000;
	v[0] = v[0] + v[0];
	u32 volatile* a= (u32 volatile*)0xfec00000;
	a[0xB0/4]=0;


	asm("iretq;");
}
void irq1() {
	asm("mov $0xdead0001,%eax;");
	asm("jmp .;");
}
void irq2() {
	asm("mov $0xdead0002,%eax;");
	asm("jmp .;");
}
void irq3() {
	asm("mov $0xdead0003,%eax;");
	asm("jmp .;");
}
void irq4() {
	asm("mov $0xdead0004,%eax");
	asm("jmp .;");
}
void irq5() {
	asm("mov $0xdead0005,%eax");
	asm("jmp .;");
}
void irq6() {
	asm("mov $0xdead0006,%eax");
	asm("jmp .;");
}
void irq7() {
	asm("mov $0xdead0007,%eax");
	asm("jmp .;");
}
void irq8() {
	asm("mov $0xdead0008,%eax");
	asm("jmp .;");
}

void irq9() {
	asm("mov $0xdead0009,%eax");
	asm("jmp .;");
}

void irq10() {
	asm("mov $0xdead000a,%eax");
	asm("jmp .;");
}

void irq11() {
	asm("mov $0xdead000b,%eax");
	asm("jmp .;");
}

void irq12() {
	asm("mov $0xdead000c,%eax");
	asm("jmp .;");
}

void irq13() {
	asm("mov $0xdead000d,%eax");
	asm("jmp .;");
}

void irq14() {
	asm("mov $0xdead000e,%eax");
	asm("jmp .;");
}

void irq15() {
	asm("mov $0xdead000f,%eax");
	asm("jmp .;");
}

void irq16() {
	asm("mov $0xdead0010,%eax");
	asm("jmp .;");
}

void irq17() {
	asm("mov $0xdead0011,%eax");
	asm("jmp .;");
}

void irq18() {
	asm("mov $0xdead0012,%eax");
	asm("jmp .;");
}

void irq19() {
	asm("mov $0xdead0013,%eax");
	asm("jmp .;");
}

void irq20() {
	asm("mov $0xdead0014,%eax");
	asm("jmp .;");
}

void irq21() {
	asm("mov $0xdead0015,%eax");
	asm("jmp .;");
}

void irq22() {
	asm("mov $0xdead0016,%eax");
	asm("jmp .;");
}
void irq23() {
	asm("mov $0xdead0017,%eax");
	asm("jmp .;");
}

u64 irqs []= {
			(u64)&irq0, (u64)&irq1, (u64)&irq2, (u64)&irq3, (u64)&irq4, (u64)&irq5, (u64)&irq6, (u64)&irq7, (u64)&irq8,(u64) &irq9, (u64)&irq10,
			(u64)&irq11, (u64)&irq12, (u64)&irq13, (u64)&irq14, (u64)&irq15, (u64)&irq16, (u64)&irq17, (u64)&irq18, (u64)&irq19,
			(u64)&irq20, (u64)&irq21, (u64)&irq22, (u64)&irq23};


void scheduleTest() {

	asm("mov $0xb00fb00f,%eax");
	asm("jmp .;");
}

void setupInterrupts() {
	IDT.Limit = sizeof(IDTDescriptor)*256-1;
	IDT.BaseAddress = (u64)&IDTTable;
	u64 irqFunction;

	for(int i=0;i<256;i++)
	{
		if(i>31 && i < 56) 
			irqFunction = irqs[i-32];
		else
			irqFunction = (u64)&interruptTest;
	
		IDTTable[i].offsetLow = (u16) (irqFunction&0xffff);
		IDTTable[i].selector = 8;
		IDTTable[i].typeAttributes = 0x80 | 0xe;		// present-bit, interrupt gate
		IDTTable[i].offsetMiddle = (u16) (irqFunction>>16);
		IDTTable[i].offsetHigh = (u32) (irqFunction>>32);
/*
		IDTTable[i].offsetLow = (u16) ((u64)(&interruptTest)&0xffff);
		IDTTable[i].selector = 8;
		IDTTable[i].typeAttributes = 0x80 | 0xe;		// present-bit, interrupt gate
		IDTTable[i].offsetMiddle = (u16) ((u64)(&interruptTest)>>16);
		IDTTable[i].offsetHigh = (u32) ((u64)(&interruptTest)>>32);
*/
	}
	asm("lidt %0" : : "m" (IDT));
	//asm("sti;");
	//		asm("int $80;");
}
