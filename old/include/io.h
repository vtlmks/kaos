#ifndef __IO_H__
#define __IO_H__

static inline u8 out8(u16 port, u8 value) {
	asm volatile("outb %1, %0" : : "dN" (port), "a" (value));
	return value;
}

static inline u16 out16(u16 port, u16 value) {
	asm volatile("outw %1, %0" : : "dN" (port), "a" (value));
	return value;
}

static inline u32 out32(u16 port, u32 value) {
	asm volatile("outl %1, %0" : : "dN" (port), "a" (value));
	return value;
}

static inline u8 in8(u16 port) {
	u8 result;
	asm volatile("inb %1, %0" : "=a" (result) : "dN" (port));
	return result;
}

static inline u16 in16(u16 port) {
	u16 result;
	asm volatile("inw %1, %0" : "=a" (result) : "dN" (port));
	return result;
}

static inline u32 in32(u16 port) {
	u32 result;
	asm volatile("inl %1, %0" : "=a" (result) : "dN" (port));
	return result;
}


#endif
