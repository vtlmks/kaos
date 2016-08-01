#include "include\assembler.h"
#include "include\types.h"


#define PIT_COMMAND_REGISTER					0x43		// WO

#define PITCMD_CHANNEL2							0x80
#define PITCMD_ACCESSMODE_LOHI					0x30
#define PITCMD_ONESHOT							0x2


#define PIT_CHANNEL0							0x40		// RW
#define PIT_CHANNEL1							0x41		// RW
#define PIT_CHANNEL2							0x42		// RW



#define KB8042_DATAPORT							0x60		// RW
#define KB8042_DATAPORT2						0x61
#define KB8042_STATUS_REGISTER					0x64		// RW


#define IA32_APIC_BASE_MSR 						0x1B
#define IA32_APIC_BASE_MSR_BSP 					0x100 		// Processor is a BSP

#define APIC_GLOBAL_ENABLE_FLAG					(1<<11)		// APIC Global enable flag
#define APIC_NMI								(4<<8)
#define APIC_SW_ENABLE							(1<<8)
#define APIC_DISABLE							(1<<16)
#define APIC_TIMER_PERIODIC						(1<<17)

#define	APIC_ID_REGISTER						0x20/4		// RW
#define APIC_VERSION_REGISTER					0x30/4		// RO
#define APIC_TASK_PRIORITY_REGISTER				0x80/4
#define APIC_EOI_REGISTER						0xB0/4		// WO
#define APIC_LDR								0XD0/4
#define APIC_DFR								0xE0/4
#define APIC_SPURIOUS_INTERRUPT_VECTOR_REGISTER	0xF0/4		// RW
#define APIC_ERROR_STATUS_REGISTER				0x280/4		// RO?
#define APIC_INTERRUPT_REGISTER_LOW				0x300/4		// RW
#define APIC_INTERRUPT_REGISTER_HIGH			0x310/4		// RW
#define APIC_LVT_TIMER_REGISTER					0x320/4		// RW
#define APIC_LVT_PERF							0x340/4
#define APIC_LVT_LINT0							0x350/4
#define APIC_LVT_LINT1							0x360/4
#define APIC_LVT_ERROR							0x370/4
#define APIC_INITIAL_COUNT_REGISTER				0x380/4		// RW
#define APIC_CURRENT_COUNT_REGISTER				0x390/4		// RO
#define APIC_DIVIDE_CONFIGURATION_REGISTER		0x3E0/4		// RW

// I/O Window Register, offset from IOAPIC base
#define IOAPIC_IOREGSEL							0x0/4		// RW		
#define IOAPIC_IOWIN							0x10/4		// RW

// IOAPIC Registers
#define IOAPICID								0x0
#define IOAPICVER								0x1
#define IOAPICARB								0x2
#define IOREDTBL0								0x10 //	0x10-0x3F 24 64-bit I/O Redirection table entry Registers

int kprintf(const char *formatString, ...);

void setupIOAPIC() {

	// todo get saved IOAPIC address from ACPI

	u32 volatile* ioapicBase = (u32 volatile *)0xfec00000;
	ioapicBase[IOAPIC_IOREGSEL] = IOAPICVER;
	u32 apicver = ioapicBase[IOAPIC_IOWIN];
	u8 version = apicver & 0xff;
	u8 irqs = ((apicver >> 16) & 0xff) + 1;
	kprintf("IOAPIC version:%d irqs:%d %08x\n",version, irqs, apicver);

	/*
	for(u8 irq=0;irq<irqs;irq++) {

		if(irq==2 || irq == 13)
			continue;

		ioapicBase[IOAPIC_IOREGSEL] = IOREDTBL0+2*irq;
		ioapicBase[IOAPIC_IOWIN] = irq + 32;
		ioapicBase[IOAPIC_IOREGSEL] = IOREDTBL0+2*irq+1;
		ioapicBase[IOAPIC_IOWIN] = 0 << 24;		// todo apicid

		//kprintf("REDTBL data %08x %08x\n", tbl0a, tbl0b);
	}
	*/
}

void setupApic() {
	u64 apic = rdmsr(IA32_APIC_BASE_MSR) & 0xfffff000;					// bit 8=Processor is BSP, bit 11=global enable/disable
	u32 volatile *apicBase = (u32 volatile *)apic;
	u32 currentCount;
	
	//kprintf("apicbase = %08x\n", apicBase);
	
	// reset to initial state
	apicBase[APIC_DFR] = 0xFFFFFFFF;
	u32 ldr = apicBase[APIC_LDR];
	ldr &= 0xFFFFFF;
	ldr |= 1;

	apicBase[APIC_LDR] = ldr;
	apicBase[APIC_LVT_TIMER_REGISTER] = APIC_DISABLE;
	apicBase[APIC_LVT_PERF] = APIC_NMI;
	apicBase[APIC_LVT_LINT0] = APIC_DISABLE;	// not disabled on bsp?
	apicBase[APIC_LVT_LINT1] = APIC_DISABLE;	// NMI
	apicBase[APIC_TASK_PRIORITY_REGISTER] = 0;

	// enable APIC
	apic |= APIC_GLOBAL_ENABLE_FLAG;
	wrmsr(IA32_APIC_BASE_MSR, apic);

	apicBase[APIC_SPURIOUS_INTERRUPT_VECTOR_REGISTER]=32 + 7 + APIC_SW_ENABLE;		// SIV = IRQ7
	apicBase[APIC_LVT_TIMER_REGISTER] = 32+0;										// LWT = IRQ0
	apicBase[APIC_DIVIDE_CONFIGURATION_REGISTER] = 3;								// Divide value = 16

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

	apicBase[APIC_INITIAL_COUNT_REGISTER] = 0xffffffff;		// start from -1
	while((in(KB8042_DATAPORT2) & 0x20) != 0);
	apicBase[APIC_LVT_TIMER_REGISTER] = APIC_DISABLE;			// stop
	u64 ticks = ((0xFFFFFFFF - apicBase[APIC_CURRENT_COUNT_REGISTER])+1)*16000;
	//currentCount *=16000;
	
	kprintf("currentcount %016x",ticks);
	
	// 0xfee01003
	//currentCount *=16;
	//currentCount *=100;

	apicBase[APIC_INITIAL_COUNT_REGISTER] =5555; //(u32)ticks;
	apicBase[APIC_LVT_TIMER_REGISTER] = (32 | APIC_TIMER_PERIODIC);
	apicBase[APIC_DIVIDE_CONFIGURATION_REGISTER] = 0xb; //3;								// Divide value = 16
	apicBase[APIC_EOI_REGISTER] = 0;
	//apicBase[APIC_ERROR_STATUS_REGISTER]=0;
	//apicBase[APIC_ERROR_STATUS_REGISTER]=0;
	//apicBase[APIC_LVT_ERROR] = 19+32;

	//apicBase[APIC_INTERRUPT_REGISTER_HIGH]=0;
	//apicBase[APIC_INTERRUPT_REGISTER_LOW] = (0x500|0x8000|0x80000);

	//while(apicBase[APIC_INTERRUPT_REGISTER_LOW] & 0x1000);

	//apicBase[APIC_TASK_PRIORITY_REGISTER]=0;

	//apicBase[APIC_EOI_REGISTER] = 0;


	// done
	asm volatile("mov $0xdeadf00d,%eax;");
	//asm volatile("sti;");
	asm volatile("jmp .;");

	// ax = fee00900
	//u64 msrAdress;
	//char str[256];
	//print(str, "APIC base adress = %X", msrAdress);
}
