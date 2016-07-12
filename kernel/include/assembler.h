#ifndef __ASSEMBLER_H_
#define __ASSEMBLER_H_

#include "types.h"
//#define wrmsr(ecx,eax,edx) asm volatile ("wrmsr" : : "c" (ecx), "a" (eax), "d" (edx))
//#define rdmsr(ecx,eax,edx) __asm volatile ("rdmsr" : "=a" (eax), "=d" (edx) : "c" (ecx))
//
//inline void outb(u16 port, u8 value) {
//	asm("		mov %al, value"
//		"mov %dx, port"
//		"out %dx, %al"
//	);
//}

#endif

