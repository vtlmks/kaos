
/*
** Some warnings:
**  Structure line alignment is changed from default to a 1 byte alignment; (normally it is 8 or 16 byte)
**
**  All around rusty assembly, but it'll be fine...  mostly...
*/

#include "include\assembler.h"
#include "include\types.h"
#include "tty.h"
#include <loaderinfo.h>
//#include <intrin.h>

extern "C" void kernelmain(LoaderInfo *info);

void interruptsInit();
void apicInit();

void kernelmain(LoaderInfo *info) __attribute__((section(".kernelmain")));
void kernelmain(LoaderInfo *info)  {
	//(".intel_syntax noprefix");
//	asm("mov	$0xdeadbeef,%rax;");
	ttyInit(info);
	interruptsInit();
	apicInit();
	asm("jmp .;");

//		"cli;"
//		"mov	%ax,0x10;"
//		"mov	%ax,%ds;"
//		"mov	%ax,%es;"
//		"mov	%ax,%fs;"
//		"mov	%ax,%gs;"
//		"mov	%ax,%ss;"
//		"mov	$80000,%esp;"
//		"mov	%esp,%ebp;"
//		//"push	%ebp;"
//		"sti;"
//	);
}
