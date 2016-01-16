/* vim: tabstop=3 shiftwidth=4 noexpandtab:
**
**
**
*/

#if !defined(__cplusplus)
#include <stdbool.h>
#endif

#include <stddef.h>
#include <stdint.h>

#include "psf.h"
#include "fontLat2Terminus16.h"

// #if defined(__linux__)
// #error "Wrong compiler"
// #endif
//
// #if !defined(__i386__)
// #error "x86-elf compiler please"
// #endif

// TODO(peter): mokaos

/* remove later.
enum vga_color {
	COLOR_BLACK = 0,
	COLOR_BLUE = 1,
	COLOR_GREEN = 2,
	COLOR_CYAN = 3,
	COLOR_RED = 4,
	COLOR_MAGENTA = 5,
	COLOR_BROWN = 6,
	COLOR_LIGHT_GREY = 7,
	COLOR_DARK_GREY = 8,
	COLOR_LIGHT_BLUE = 9,
	COLOR_LIGHT_GREEN = 10,
	COLOR_LIGHT_CYAN = 11,
	COLOR_LIGHT_RED = 12,
	COLOR_LIGHT_MAGENTA = 13,
	COLOR_LIGHT_BROWN = 14,
	COLOR_WHITE = 15,
};
*/

struct VbeModeInfoBlock {
	// Mandatory information for all VBE revisions
	uint16_t	ModeAttributes;
	uint8_t	WinAAttributes;
	uint8_t	WinBAttributes;
	uint16_t	WinGranularity;
	uint16_t	WinSize;
	uint16_t	WinASegment;
	uint16_t	WinBSegment;
	uint32_t	WinFuncPtr;
	uint16_t	BytesPerScanLine;

	// Mandatory information for VBE 1.2 and above
	uint16_t	XResolution;
	uint16_t	YResolution;
	uint8_t	XCharSize;
	uint8_t	YCharSize;
	uint8_t	NumberOfPlanes;
	uint8_t	BitsPerPixel;
	uint8_t	NumberOfBanks;
	uint8_t	MemoryModel;
	uint8_t	BankSize;
	uint8_t	NumberOfImagePages;
	uint8_t	Reserved_page;

	// Direct Color fields (required for direct/6 and YUV/7 memory models)
	uint8_t	RedMaskSize;
	uint8_t	RedFieldPosition;
	uint8_t	GreenMaskSize;
	uint8_t	GreenFieldPosition;
	uint8_t	BlueMaskSize;
	uint8_t	BlueFieldPosition;
	uint8_t	RsvdMaskSize;
	uint8_t	RsvdFieldPosition;
	uint8_t	DirectColorModeInfo;

	// Mandatory information for VBE 2.0 and above
	uint32_t	PhysBasePtr;
	uint32_t	OffScreenMemOffset;
	uint16_t	OffScreenMemSize;

	// Mandatory information for VBE 3.0 and above
	uint16_t	LinBytesPerScanLine;
	uint8_t	BnkNumberOfPages;
	uint8_t	LinNumberOfPages;
	uint8_t	LinRedMaskSize;
	uint8_t	LinRedFieldPosition;
	uint8_t	LinGreenMaskSize;
	uint8_t	LinGreenFieldPosition;
	uint8_t	LinBlueMaskSize;
	uint8_t	LinBlueFieldPosition;
	uint8_t	LinRsvdMaskSize;
	uint8_t	LinRsvdFieldPosition;
	uint32_t	MaxPixelClock;
	uint8_t	Reserved[189];
};

struct VbeInfoBlock {
	uint8_t	VbeSignature[4];
	uint16_t	VbeVersion;
	uint16_t	OemStringPtr_Off;
	uint16_t	OemStringPtr_Seg;
	uint8_t	Capabilities[4];
	uint16_t	VideoModePtr_Off;
	uint16_t	VideoModePtr_Seg;
	uint16_t	TotalMemory;
	uint16_t	OemSoftwareRev;
	uint16_t	OemVendorNamePtr_Off;
	uint16_t	OemVendorNamePtr_Seg;
	uint16_t	OemProductNamePtr_Off;
	uint16_t	OemProductNamePtr_Seg;
	uint16_t	OemProductRevPtr_Off;
	uint16_t	OemProductRevPtr_Seg;
	uint16_t	Reserved[111]; // used for dynamicly generated mode list
	uint8_t	OemData[256];
};

struct MultibootInfo {
	uint32_t	         flags;
	uint32_t	         memLower;
	uint32_t	         memUpper;
	uint32_t	         bootDevice;
	uint32_t	         cmdLine;
	uint32_t	         modsCount;
	uint32_t	         modsAddr;
	uint32_t	         syms[4];
	uint32_t	         mmapLength;
	uint32_t	         mmapAddr;
	uint32_t	         drivesLength;
	uint32_t	         drivesAddr;
	uint32_t	         configTable;
	uint32_t	         bootLoaderName;
	uint32_t	         apmTable;
	VbeInfoBlock		*vbeInfoBlock;
	VbeModeInfoBlock	*vbeModeInfoBlock;
	uint16_t			   vbeMode;
	uint16_t			   vbeInterfaceSegment;
	uint16_t			   vbeInterfaceOffset;
	uint16_t			   vbeInterfaceLength;
};

struct vec2 {
	uint8_t	x;
	uint8_t	y;
};

vec2 cursor				= {0,0};
uint32_t	frontColor	= 0xfff0a438;
uint32_t	backColor	= 0xff0c1420;

VbeModeInfoBlock	*modeInfo;
uint32_t					*frameBuffer;
PSF2							*psf;
uint8_t						*font;


size_t strlen(const char *str) {
	size_t result = 0;
	while(*str++) {
 		result++;
	}
	return result;
}

void writeString(const char *stringBuffer, vec2 *cur = &cursor) {
	while(uint8_t character = *stringBuffer++) {
		if(character == '\n') {
			cur->y++;
			cur->x = 0;
		} else {
			for(uint32_t row = 0; row < psf->height; ++row) { //psf->height; ++y) {
				uint16_t fontRowData = font[(character * psf->charSize) + row];
				for(uint32_t pixel = 0; pixel < psf->width; ++pixel) {
					if (fontRowData & 1 << pixel) {
						frameBuffer[cur->x*8 + ((row + (cur->y*psf->height)) * 1280) + (8 - pixel)] = frontColor;
					}
				};
			};
			cur->x++;
		}
	}
}

void writeHex(uint32_t value) {
	static const char hexChars[] = "0123456789ABCDEF";
	char temp[9];
	for (size_t i = 0, j = (8-1)*4 ; i<8; ++i, j -= 4) {
		temp[i] = hexChars[(value >> j) & 0xf];
	}
	temp[8] = 0;
	writeString(temp);
}


#if defined(__cplusplus)
extern "C"
#endif
void kernelMain(MultibootInfo *m) {
//	termInit();


//	uint32_t *ptr = (uint32_t *)0x40000;		// so we can write stuff and read out from qemu -monitor stdio
//	ptr[0] = strlen(stringBuffer);

	modeInfo		= m->vbeModeInfoBlock;
	frameBuffer	= (uint32_t *)modeInfo->PhysBasePtr;
	psf					= (PSF2 *)&fontLat2Terminus16;
	font				= (uint8_t *)(psf) + psf->headerSize;

	for(size_t i = 0; i < 1280*720; ++i) {	// clear screen
		frameBuffer[i] = backColor;
	}

	cursor.x = 0;
	cursor.y = 0;
	frontColor	= 0xfff0a438;

	writeString("    KAOS v0.0.0 - Created by Mindkiller Systems.\n"
    			"    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
				);

	vec2 pos = {30, 30};
	writeString("This is a test\n", &pos);;
	writeString("This is a test 0x");
	writeHex(0x4748494a);
	writeString("\n");

//	while(1) {
	// for(size_t i = 0; i < 1280*720; ++i) {	// clear screen
	// 	frameBuffer[i] = backColor;
	// }

}
