#pragma once


struct e820Entry {
	u64	addr;
	u64	size;
	u32	type;
} __attribute__ ((__packed__));

