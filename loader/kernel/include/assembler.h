#ifndef __ASSEMBLER_H_
#define __ASSEMBLER_H_

#include "types.h"

inline void outb(u16 port, u8 value) {
	__asm {
		mov al, value
		mov dx, port
		out dx, al
	}
}

#endif

