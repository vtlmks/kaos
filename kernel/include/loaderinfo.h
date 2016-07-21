#pragma once

#include <types.h>
#include <meminfo.h>

struct __attribute__ ((__packed__)) LoaderInfo {
	memInfo	*memInfoPtr;
	u32		*vesaPhysBasePtr;
	u16		vesaBytesPerRow;
	u16		vesaPixelWidth;
	u16		vesaPixelHeight;
	u8			memInfoCount;
	u8			vesaPixelDepth;
};

