

#include <loaderinfo.h>
#include <e820.h>

void e820(LoaderInfo *info) {
	e820Entry *biosMem = info->e820Ptr;
	e820Entry newMem[128];

	u8 biosMemCount = info->e820Count;
	u8 newMemCount = 0;

	for(u8 i = 0; i < biosMemCount; ++i) {
//		if(biosMem[i]->addr
	}

}
