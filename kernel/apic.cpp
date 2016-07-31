#include "include\assembler.h"
#include "include\types.h"


#define PIT_COMMAND_REGISTER	0x43		// WO

#define PITCMD_CHANNEL2				0x80
#define PITCMD_ACCESSMODE_LOHI	0x30
#define PITCMD_ONESHOT					0x2


#define PIT_CHANNEL0			0x40		// RW
#define PIT_CHANNEL1			0x41		// RW
#define PIT_CHANNEL2			0x42		// RW



#define KB8042_DATAPORT			0x60		// RW
#define KB8042_DATAPORT2		0x61
#define KB8042_STATUS_REGISTER	0x64		// RW


#define IA32_APIC_BASE_MSR 						0x1B
#define IA32_APIC_BASE_MSR_BSP 					0x100 // Processor is a BSP

#define APIC_GLOBAL_ENABLE_FLAG					1<<11			// APIC Global enable flag
#define APIC_NMI								4<<8
#define APIC_SW_ENABLE							0x100
#define APIC_DISABLE							0x10000


#define	APIC_ID_REGISTER						0x20		// RW
#define APIC_VERSION_REGISTER					0x30		// RO
#define APIC_TASK_PRIORITY_REGISTER				0x80
#define APIC_EOI_REGISTER						0xB0		// WO
#define APIC_LDR								0XD0
#define APIC_DFR								0xE0
#define APIC_SPURIOUS_INTERRUPT_VECTOR_REGISTER	0xF0		// RW
#define APIC_LVT_TIMER_REGISTER					0x320		// RW
#define APIC_LVT_PERF							0x340
#define APIC_LVT_LINT0							0x350
#define APIC_LVT_LINT1							0x360
#define APIC_LVT_ERROR							0x370
#define APIC_INITIAL_COUNT_REGISTER				0x380		// RW
#define APIC_CURRENT_COUNT_REGISTER				0x390		// RO
#define APIC_DIVIDE_CONFIGURATION_REGISTER		0x3E0		// RW

// I/O Window Register, offset from IOAPIC base
#define IOAPIC_IOREGSEL							0x0/4		// RW		
#define IOAPIC_IOWIN							0x10/4		// RW

// IOAPIC Registers
#define IOAPICID	0x0
#define IOAPICVER	0x1
#define IOAPICARB	0x2
#define IOREDTBL0	0x10 //	0x10-0x3F 24 64-bit I/O Redirection table entry Registers




//#define rdmsr(msr,val1,val2) asm volatile("rdmsr" : "=a" (val1), "=d" (val2) : "c" (msr)); // __asm  ("rdmsr" : "=a"(eax), "=d"(edx) : "c"(ecx));
//#define rdmsr(msr, lo, hi) asm volatile("rdmsr" : "=a"(*lo), "=d"(*hi) : "c"(msr));

int kprintf(const char *formatString, ...);

u32 currentCount;

void setupIOAPIC() {

//	asm("mov $0xdeadbeef,%eax;");
//	asm("jmp .;");

	// todo get saved IOAPIC address from ACPI
/*
	u32 ioapicBase = 0xfec00000;
	*(u32*)(ioapicBase+IOAPIC_IOREGSEL) = IOAPICVER;
	u32 apicver = *(u32*)(ioapicBase+IOAPIC_IOWIN);
*/
	u32* ioapicBase = (u32*)0xfec00000;
	*(u32*)(ioapicBase+IOAPIC_IOREGSEL) = IOAPICVER;
	u32 apicver = *(u32*)(ioapicBase+IOAPIC_IOWIN);
	u8 version = apicver & 0xff;
	u8 irqs = ((apicver >> 16) & 0xff) + 1;
	kprintf("IOAPIC version:%d irqs:%d %08x\n",version, irqs, apicver);
//		*(u32*)(ioapicBase+IOAPIC_IOREGSEL) = 0x10; //IOREDTBL0+2*irq;
//		*(u32*)(ioapicBase+IOAPIC_IOREGSEL) = 0x10; //IOREDTBL0+2*irq;
//		*(u32*)(ioapicBase+IOAPIC_IOREGSEL) = 0x10; //IOREDTBL0+2*irq;

//		*(u32*)(ioapicBase+IOAPIC_IOREGSEL) = 0x10;RGB(0x00,0x00,0x00)RGB(0x00,0x00,0x00)RGB(0x00,0x00,0x00)RGB(0x00,0x00,0x00)

	for(u8 irq=0;irq<irqs;irq++) {
		*(u32*)(ioapicBase+IOAPIC_IOREGSEL) = IOREDTBL0+2*irq;
		u32 tbl0a = *(u32*)(ioapicBase+IOAPIC_IOWIN);

		*(u32*)(ioapicBase+IOAPIC_IOREGSEL) = IOREDTBL0+2*irq+1;
		u32 tbl0b = *(u32*)(ioapicBase+IOAPIC_IOWIN);

		kprintf("REDTBL data %08x %08x\n", tbl0a, tbl0b);
	}
}

void setupApic() {
	u64 apic = rdmsr(IA32_APIC_BASE_MSR) & 0xfffff100;					// bit 8=Processor is BSP, bit 11=global enable/disable

	// reset to initial state
	*((u32*) (apic+APIC_DFR)) = 0xFFFFFFFF;
	u32 ldr = *((u32*) (apic+APIC_LDR));
	ldr &= 0xFFFFFF;
	ldr |= 1;
	*((u32*) (apic+APIC_LDR)) = ldr;
	*((u32*) (apic+APIC_LVT_TIMER_REGISTER)) = APIC_DISABLE;
	*((u32*) (apic+APIC_LVT_PERF)) = APIC_NMI;
	*((u32*) (apic+APIC_LVT_LINT0)) = APIC_DISABLE;
	*((u32*) (apic+APIC_LVT_LINT1)) = APIC_DISABLE;
	*((u32*) (apic+APIC_TASK_PRIORITY_REGISTER)) = 0;

	// enable APIC
	apic |= APIC_GLOBAL_ENABLE_FLAG;
	wrmsr(IA32_APIC_BASE_MSR, apic);

	*((u32*) (apic+APIC_SPURIOUS_INTERRUPT_VECTOR_REGISTER))=32 + 7 + APIC_SW_ENABLE;			// SIV = IRQ7
	*((u32*) (apic+APIC_LVT_TIMER_REGISTER)) = 32 + 0;										// LWT = IRQ0
	*((u32*) (apic+APIC_DIVIDE_CONFIGURATION_REGISTER)) = 3;									// Divide value = 16

	// get frequency
	u8 r = (in(KB8042_DATAPORT2) & 0xfd) | 1;
	out(KB8042_DATAPORT2, r);
	out(PIT_COMMAND_REGISTER, PITCMD_CHANNEL2|PITCMD_ACCESSMODE_LOHI|PITCMD_ONESHOT);

	// 1193180/100 Hz = 11931 = 0x2e9b
	out(PIT_CHANNEL2, 0x9b);
	in(KB8042_DATAPORT);			// delay
	out(PIT_CHANNEL2, 0x2e);

	// clear and set bit 0 of IO port 0x61 before reloading PIT one-shot counter
	u8 oneshot = in(KB8042_DATAPORT2) & ~1;
	out(KB8042_DATAPORT2, oneshot);
	oneshot |= 1;									// enable PIT channel 2 gate
	out(KB8042_DATAPORT2, oneshot);

	*((u32*) (apic+APIC_INITIAL_COUNT_REGISTER))= 0xffffffff;		// start from -1


	currentCount = *((u32*) (apic+APIC_INITIAL_COUNT_REGISTER));
	kprintf("currentcount %08x",currentCount);
	

	while((in(KB8042_DATAPORT2) & 0x20) != 0) {}		

	*((u32*) (apic+APIC_LVT_TIMER_REGISTER)) = APIC_DISABLE;			// stop

	currentCount = *((u32*) (apic+APIC_CURRENT_COUNT_REGISTER));
	// 0xfee01003
	//currentCount *=16;
	//currentCount *=100;

	// done
	asm("mov currentCount,%ebx;");
	asm("mov $0xdeadbabe,%eax;");
	asm("hlt");

	// ax = fee00900
	//u64 msrAdress;
	//char str[256];
	//print(str, "APIC base adress = %X", msrAdress);
}
