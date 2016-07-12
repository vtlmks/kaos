
/*
** Some warnings:
**  Structure line alignment is changed from default to a 1 byte alignment; (normally it is 8 or 16 byte)
**
**  All around rusty assembly, but it'll be fine...  mostly...
*/

#include "include\assembler.h"
#include "include\types.h"
//#include <intrin.h>


char const *loaderWelcome = "Second stage loader started";
char const *loaderInitializing = " [  ] Initializing memory, setting up structures";
char const *loaderMemoryprobe = " [  ] Trying to find out how much memory this machine have available";
char const *loaderSwitchingvideomode = " [  ] Switching to something better than 80x25";
char const *loaderMessage = " [  ] Loading kernel";
char const *loaderOK = " [OK]";

extern "C" void kernelmain();

void apicInit();

void kernelmain() {

	//(".intel_syntax noprefix");
	//asm("mov	$deadbeef,%rax;"
	asm("jmp	.;"
		"cli;"
		"mov	%ax,0x10;"
		"mov	%ax,%ds;"
		"mov	%ax,%es;"
		"mov	%ax,%fs;"
		"mov	%ax,%gs;"
		"mov	%ax,%ss;"
		"mov	$80000,%esp;"
		"mov	%esp,%ebp;"
		//"push	%ebp;"
		"sti;"
	);

	apicInit();

	//asm("jmp $");

	char *a = (char *) loaderWelcome;


	volatile int b = 0;

	for(unsigned int i = 0; i < 10; ++i) {
		a[i] == 0 ? b = 1 : b = 0;
	}

	if(b) a[2] = 0;
}

#define IA32_APIC_BASE_MSR 0x1B
#define IA32_APIC_BASE_MSR_BSP 0x100 // Processor is a BSP
#define IA32_APIC_BASE_MSR_ENABLE 0x800

//#define rdmsr(msr,val1,val2) asm volatile("rdmsr" : "=a" (val1), "=d" (val2) : "c" (msr)); // __asm  ("rdmsr" : "=a"(eax), "=d"(edx) : "c"(ecx));
//#define rdmsr(msr, lo, hi) asm volatile("rdmsr" : "=a"(*lo), "=d"(*hi) : "c"(msr));


void apicInit() {
	//__readmsr(0x1b);
}
