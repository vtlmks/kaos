

#include <loaderinfo.h>
#include <meminfo.h>

void e820(LoaderInfo *info) {
	MemInfo *biosMem = info->memInfoPtr;
	u8 biosMemCount = info->memInfoCount;
	MemInfo newMem[128];

	u8 newMemCount = 0;

	for(u8 i = 0; i < biosMemCount; ++i) {
//		if(biosMem[i]->from 
	}

}
