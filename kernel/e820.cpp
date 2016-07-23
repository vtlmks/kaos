

#include <loaderinfo.h>
#include <e820.h>

int kprintf(const char *formatString, ...);

const char *memType[] = {
	"type % u",
	"usable",
	"reserved",
	"ACPI data",
	"ACPI NVS",
	"unusable",
	"persistent (type % u)"
};

void e820(LoaderInfo *info) {
	e820Entry *e820Mem = info->e820Ptr;
	e820Entry newMem[128];

	kprintf("BIOS-provided physical RAM map:\n");
	for(u8 i = 0; i < info->e820Count; ++i) {
		kprintf(" BIOS-e820: %016x - 0x%016x (%s)\n", e820Mem[i].addr, e820Mem[i].size, memType[e820Mem[i].type]);
	}

	u8 biosMemCount = info->e820Count;
	u8 newMemCount = 0;

	for(u8 i = 0; i < biosMemCount; ++i) {
//		if(biosMem[i]->addr
	}

}
