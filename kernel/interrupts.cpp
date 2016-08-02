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

struct LIDTEntry {
	u16	Limit;
	u64	BaseAddress;
} __attribute((__packed__));

LIDTEntry		IDT;
IDTDescriptor	IDTTable[256];	
u64 			dummy;
u64 			dummy2;


int kprintf(const char *formatString, ...);

void exception00() { asm("mov $0xC0DE0000,%eax;"); kprintf("\n#DE\n"); asm("hlt;"); }
void exception01() { asm("mov $0xC0DE0001,%eax;"); kprintf("\n#DB\n"); asm("hlt;"); }
void exception02() { asm("mov $0xC0DE0002,%eax;"); kprintf("\n#NMI\n"); asm("hlt;"); }
void exception03() { asm("mov $0xC0DE0003,%eax;"); kprintf("\n#BP\n"); asm("hlt;"); }
void exception04() { asm("mov $0xC0DE0004,%eax;"); kprintf("\n#OF\n"); asm("hlt;"); }
void exception05() { asm("mov $0xC0DE0005,%eax;"); kprintf("\n#BR\n"); asm("hlt;"); }
void exception06() { asm("mov $0xC0DE0006,%eax;"); kprintf("\n#UD\n"); asm("hlt;"); }
void exception07() { asm("mov $0xC0DE0007,%eax;"); kprintf("\n#NM\n"); asm("hlt;"); }
void exception08() { asm("mov $0xC0DE0008,%eax;"); kprintf("\n#DF\n"); asm("hlt;"); }
void exception09() { asm("mov $0xC0DE0009,%eax;"); kprintf("\n#CP\n"); asm("hlt;"); }
void exception10() { asm("mov $0xC0DE000A,%eax;"); kprintf("\n#TS\n"); asm("hlt;"); }
void exception11() { asm("mov $0xC0DE000B,%eax;"); kprintf("\n#NP\n"); asm("hlt;"); }
void exception12() { asm("mov $0xC0DE000C,%eax;"); kprintf("\n#SS\n"); asm("hlt;"); }
void exception13() { asm("mov $0xC0DE000D,%eax;"); kprintf("\n#GP\n"); asm("hlt;"); }
void exception14() { asm("mov $0xC0DE000E,%eax;"); kprintf("\n#PF\n"); asm("hlt;"); }
void exception15() { asm("mov $0xC0DE000F,%eax;"); kprintf("\n#--\n"); asm("hlt;"); }
void exception16() { asm("mov $0xC0DE0010,%eax;"); kprintf("\n#MF\n"); asm("hlt;"); }
void exception17() { asm("mov $0xC0DE0011,%eax;"); kprintf("\n#AC\n"); asm("hlt;"); }
void exception18() { asm("mov $0xC0DE0012,%eax;"); kprintf("\n#MC\n"); asm("hlt;"); }
void exception19() { asm("mov $0xC0DE0013,%eax;"); kprintf("\n#XM\n"); asm("hlt;"); }
void exception20() { asm("mov $0xC0DE0014,%eax;"); kprintf("\n#VE\n"); asm("hlt;"); }

void interruptTest() {
	asm("mov $0xdeadffff,%eax;");
	asm("jmp .;");
}

void irq0() {
	kprintf("mokaw\n");
//	u32* v = (u32*)0xfd000000;
//	v[0] = v[0] + v[0];
//	u32 volatile* a= (u32 volatile*)0xfec00000;
	//a[0xB0/4]=0;
	
	asm("pushq $0x7aba7aba;");
	
	//asm("jmp .;");
	
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

u64 exceptions[] = {
	(u64)exception00,(u64)exception01,(u64)exception02,(u64)exception03,(u64)exception04,(u64)exception05,(u64)exception06,
	(u64)exception07,(u64)exception08,(u64)exception09,(u64)exception10,(u64)exception11,(u64)exception12,(u64)exception13,
	(u64)exception14,(u64)exception15,(u64)exception16,(u64)exception17,(u64)exception18,(u64)exception19,(u64)exception20,
};

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
		if(i<21)
			irqFunction = exceptions[i];
		else if(i>31 && i < 56) 
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
