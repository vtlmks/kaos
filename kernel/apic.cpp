#include "include\assembler.h"
#include "include\types.h"


#define PIT_COMMAND_REGISTER	0x43		// WO

#define PITCMD_CHANNEL2			0x80
#define PITCMD_ACCESSMODE_LOHI	0x30
#define PITCMD_ONESHOT			0x2


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


//#define rdmsr(msr,val1,val2) asm volatile("rdmsr" : "=a" (val1), "=d" (val2) : "c" (msr)); // __asm  ("rdmsr" : "=a"(eax), "=d"(edx) : "c"(ecx));
//#define rdmsr(msr, lo, hi) asm volatile("rdmsr" : "=a"(*lo), "=d"(*hi) : "c"(msr));

u32 currentCount;



void apicInit() {

	u64 apic = rdmsr(IA32_APIC_BASE_MSR) & 0xfffff100;					// bit 8=Processor is BSP, bit 11=global enable/disable

	// reset to initial state
	((u64*) (apic+APIC_DFR))[0] = 0x0FFFFFFFF;
	u32 ldr = ((u64*) (apic+APIC_LDR))[0];
	ldr &= 0x00FFFFFF;
	ldr |= 1;
	((u64*) (apic+APIC_LDR))[0] = ldr;
	((u64*) (apic+APIC_LVT_TIMER_REGISTER))[0] = APIC_DISABLE;
	((u64*) (apic+APIC_LVT_PERF))[0] = APIC_NMI;
	((u64*) (apic+APIC_LVT_LINT0))[0] = APIC_DISABLE;
	((u64*) (apic+APIC_LVT_LINT1))[0] = APIC_DISABLE;
	((u64*) (apic+APIC_TASK_PRIORITY_REGISTER))[0] = 0;

	// enable APIC
	apic |= APIC_GLOBAL_ENABLE_FLAG;
	wrmsr(IA32_APIC_BASE_MSR, apic);


	((u64*) (apic+APIC_SPURIOUS_INTERRUPT_VECTOR_REGISTER))[0]=32 + 7 + APIC_SW_ENABLE;			// SIV = IRQ7
	((u64*) (apic+APIC_LVT_TIMER_REGISTER))[0] = 32 + 0;										// LWT = IRQ0
	((u64*) (apic+APIC_DIVIDE_CONFIGURATION_REGISTER))[0] = 3;									// Divide value = 16


	// this could alternatively be done with CPUID(0x15) instruction 
	// but qemu doesn't support it

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

	((u64*) (apic+APIC_INITIAL_COUNT_REGISTER))[0] = 0xffffffff;		// start from -1


	while((in(KB8042_DATAPORT2) & 0x20) != 0)							// wait for PIT

	((u64*) (apic+APIC_LVT_TIMER_REGISTER))[0] = APIC_DISABLE;			// stop

	currentCount = ((u64*) (apic+APIC_CURRENT_COUNT_REGISTER))[0];
	// 0xfee01003

	//currentCount *=16;
	//currentCount *=100;



	// done
	asm("mov currentCount,%ebx;");

	asm("mov $0xdeadbabe,%eax;");
	asm("jmp .;");







	// ax = fee00900


	//u64 msrAdress;
	//char str[256];
	//print(str, "APIC base adress = %X", msrAdress);

}
