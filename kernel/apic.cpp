#include "include\assembler.h"
#include "include\types.h"


#define PIT_COMMAND_REGISTER	0x43		// WO
#define PIT_CHANNEL0			0x40		// RW
#define PIT_CHANNEL1			0x41		// RW
#define PIT_CHANNEL2			0x42		// RW

#define KB8042_DATAPORT			0x60		// RW
#define KB8042_DATAPORT2		0x61
#define KB8042_STATUS_REGISTER	0x64		// RW

u32 currentCount;


#define IA32_APIC_BASE_MSR 						0x1B
#define IA32_APIC_BASE_MSR_BSP 					0x100 // Processor is a BSP

#define APIC_GLOBAL_ENABLE_FLAG					1<<11			// APIC Global enable flag
#define APIC_SW_ENABLE							0x100
#define APIC_DISABLE							0x10000


#define	APIC_ID_REGISTER						0x20		// RW
#define APIC_VERSION_REGISTER					0x30		// RO
#define APIC_TASK_PRIORITY_REGISTER				0x80
#define APIC_EOI_REGISTER						0xB0		// WO
#define APIC_SPURIOUS_INTERRUPT_VECTOR_REGISTER	0xF0		// RW
#define APIC_LVT_TIMER_REGISTER					0x320		// RW
#define APIC_INITIAL_COUNT_REGISTER				0x380		// RW
#define APIC_CURRENT_COUNT_REGISTER				0x390		// RO
#define APIC_DIVIDE_CONFIGURATION_REGISTER		0x3E0		// RW


//#define rdmsr(msr,val1,val2) asm volatile("rdmsr" : "=a" (val1), "=d" (val2) : "c" (msr)); // __asm  ("rdmsr" : "=a"(eax), "=d"(edx) : "c"(ecx));
//#define rdmsr(msr, lo, hi) asm volatile("rdmsr" : "=a"(*lo), "=d"(*hi) : "c"(msr));

void apicInit() {

	u64 apic = rdmsr(IA32_APIC_BASE_MSR);					// bit 8=Processor is BSP, bit 11=global enable/disable
	apic = (apic & 0xfffff100)|APIC_GLOBAL_ENABLE_FLAG;
	wrmsr(IA32_APIC_BASE_MSR, apic);

	((u64*) (apic+APIC_SPURIOUS_INTERRUPT_VECTOR_REGISTER))[0]=32 + 7 + APIC_SW_ENABLE;			// SIV = IRQ7
	((u64*) (apic+APIC_LVT_TIMER_REGISTER))[0] = 32 + 0;										// LWT = IRQ0

	u8 r = (in(KB8042_DATAPORT2) & 0xfd) | 1;
	out(KB8042_DATAPORT2, r);
	out(PIT_COMMAND_REGISTER, 0xb2);

	// 1193180/100 Hz = 11931 = 0x2e9b
	
	out(PIT_CHANNEL2, 0x9b);		// lsb
	in(KB8042_DATAPORT);			// delay
	out(PIT_CHANNEL2, 0x2e);		// msb

	// reset PIT one-shot counter
	u8 oneshot = in(KB8042_DATAPORT2) & 0xfe;
	out(oneshot, KB8042_DATAPORT2);
	oneshot |= 1;
	out(oneshot, KB8042_DATAPORT2);

	((u64*) (apic+APIC_INITIAL_COUNT_REGISTER))[0] = 0xffffffff;													// LWT = IRQ0


	while( (in(KB8042_DATAPORT2) & 0x20) != 0)

	((u64*) (apic+APIC_LVT_TIMER_REGISTER))[0] = APIC_DISABLE;													// LWT = IRQ0

	currentCount = (u32) ((u64*) (apic+APIC_CURRENT_COUNT_REGISTER))[0];

	currentCount *=16;
	currentCount *=100;

	asm("mov currentCount,%rbx;");

	asm("mov $0xdeadbabe,%eax;");
	asm("jmp .;");







	// ax = fee00900


	//u64 msrAdress;
	//char str[256];
	//print(str, "APIC base adress = %X", msrAdress);

}
