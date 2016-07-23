#pragma once

#include <types.h>
#include <e820.h>

struct LoaderInfo {
	e820Entry	*e820Ptr;
	u32			*vesaPhysBasePtr;
	u16			vesaBytesPerRow;
	u16			vesaPixelWidth;
	u16			vesaPixelHeight;
	u8			memInfoCount;
	u8			vesaPixelDepth;
} __attribute__ ((__packed__));

