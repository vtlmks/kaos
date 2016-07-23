

#include <loaderinfo.h>
#include <e820.h>

void e820(LoaderInfo *info) {
	e820entry *biosMem = info->memInfoPtr;
	u8 biosMemCount = info->memInfoCount;
	e820entry newMem[128];

	u8 newMemCount = 0;

	for(u8 i = 0; i < biosMemCount; ++i) {
		if(biosMem[i]->from 
	}

}
