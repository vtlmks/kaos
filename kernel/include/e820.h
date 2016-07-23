#pragma once


struct e820entry {
	u64	addr;
	u64	size;
	u32	type;
} __attribute__ ((__packed__));

