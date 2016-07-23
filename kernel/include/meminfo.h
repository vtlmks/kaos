#pragma once


struct __attribute__ ((__packed__)) MemInfo {
	u64	addr;
	u64	size;
	u32	type;
};
