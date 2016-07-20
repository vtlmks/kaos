#ifndef __ASSEMBLER_H_
#define __ASSEMBLER_H_

#include "types.h"
//#define wrmsr(ecx,eax,edx) asm volatile ("wrmsr" : : "c" (ecx), "a" (eax), "d" (edx))
//#define rdmsr(ecx,eax,edx) __asm volatile ("rdmsr" : "=a" (eax), "=d" (edx) : "c" (ecx))

inline u64 rdmsr(u32 msrId) {
    u64 msrValue;
    asm volatile ( "rdmsr" : "=A" (msrValue) : "c" (msrId) );
    return msrValue;
}

inline void wrmsr(u64 msrId, u64 msrValue) {
	asm volatile ( "wrmsr" : : "c" (msrId), "A" (msrValue) );
}

inline void out(u16 port, u8 value)
{
    asm volatile ( "outb %0, %1" : : "a"(value), "Nd"(port) );
}

inline u8 in(u16 port)
{
    u8 result;
    asm volatile ( "inb %1, %0"
                   : "=a"(result)
                   : "Nd"(port) );
    return result;
}

#endif

