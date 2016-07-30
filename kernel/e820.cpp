
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

struct E820SortEntry {
	e820entry	*entry;	// pointer to entry
	u64			addr;	// address of this entry
};

e820map *e820Map;
void setupE820(LoaderInfo *info) {


	e820Map->mapCount	= info->e820Count;

	// copy all info->e820Ptr to e820Map->map[]


	for(u8 i = 0; i < e820Map->mapCount; ++i) {
		e820Map->map[i].addr = info->e820Ptr[i].addr;
		e820Map->map[i].size = info->e820Ptr[i].size;
		e820Map->map[i].type = info->e820Ptr[i].type;
	}

	kprintf("BIOS-provided physical RAM map:\n");
	for(u8 j = 0; j < 200; ++j) {
	for(u8 i = 0; i < e820Map->mapCount; ++i) {
		kprintf(" BIOS-e820: %016x - 0x%016x (%s)\n", e820Map->map[i].addr, e820Map->map[i].size, memType[e820Map->map[i].type]);
	}
	}

	u8 biosMemCount = info->e820Count;
	u8 newMemCount = 0;






//	E820SortEntry	sortList[2*E820MAX];
//	E820SortEntry	*sortPointers[2*E820MAX];

//	for(u8 i = 0; i < 2 * e820Count; ++i) {
//		sortPointers[i] = &sortList[i];
//	}

//	asm("mov $e820Map, %eax");
	asm("jmp .");







	for(u8 i = 0; i < biosMemCount; ++i) {
		// sort and fix overlapping regions, higher type takes precedence

//		if(biosMem[i]->addr

//		newMem[i].addr = e820Mem[i].addr;
//		newMem[i].size = e820Mem[i].size;
//		newMem[i].type = e820Mem[i].type;
//		++newMemCount;
	}

}
