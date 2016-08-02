#pragma once

#define	E820MAX	128

struct e820entry {
	u64	addr;
	u64	size;
	u32	type;
} __attribute((packed));

struct e820map {
	u32			mapCount;
	e820entry	map[E820MAX];
};

#define	BIOS_START		0x000a0000
#define BIOS_END		0x00100000

#define	BIOS_ROM_START	0xffe00000
#define	BIOS_ROM_END	0xffffffff
