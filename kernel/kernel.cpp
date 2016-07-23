
/*
** Some warnings:
**  Structure line alignment is changed from default to a 1 byte alignment; (normally it is 8 or 16 byte)
**
**  All around rusty assembly, but it'll be fine...  mostly...
*/

#include <assembler.h>
#include <types.h>
#include <tty.h>
#include <loaderinfo.h>
//#include <intrin.h>

extern "C" void kernelmain(LoaderInfo *info);

void setupInterrupts();
void setupApic();

void setupPaging(LoaderInfo *info);

void kernelmain(LoaderInfo *info) __attribute__((section(".kernelmain")));
void kernelmain(LoaderInfo *info)  {

	asm("mov $0x10, %ax");
	asm("mov %ax, %ds");
	asm("mov %ax, %es");
	asm("mov %ax, %fs");
	asm("mov %ax, %gs");
	asm("mov %ax, %ss");
	asm("mov $0x90000, %rsp");


//	setupPaging(info);

	setupTTY(info);
	asm("jmp .;");
	setupInterrupts();
	setupApic();
	asm("jmp .;");


}

// #include <meminfo.h>


// PML4T		Equ	0x90000	; 512Gb per entry
// PDPT		Equ	0x91000	;   1Gb per entry
// PDT			Equ	0x92000	;   2Mb per entry
// PT_00000000	Equ	0x94000	;   4Kb per entry
u64 *PML4T	= (u64*)0x100000;
u64 *pdpt	= (u64*)0x101000;

u64 *kernelPD	= (u64 *)0x101000;


//void setupPaging(LoaderInfo *info) {

// 0xffffffffc0000000
// ----------------    pml4     pdpt       PDT        9          12
// 1111111111111111 111111111 111111111 000000000 000000000 000000000000


	// 1ff, 1ff, 0, 0	-> kernel (-1GB from end of memory)

	// u16 pml4	= (addr >> 39) & 0x1ff;
	// u16 PDPT	= (addr >> 30) & 0x1ff;
	// u16 PDT		= (addr >> 21) & 0x1ff;

	// memInfo mi = info->memInfoPtr;
	// u64 totalMemory = 0;

	// u32* vesaMem = info->vesaPhysBasePtr;

	// for(u8 i = 0; i < info->memInfoCount; ++i) {
		// if(mi[i].flag == 1) {
			// totalMemory += mi[i].Length;
		// }
	// }


//}
