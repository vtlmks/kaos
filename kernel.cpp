#if !defined(__cplusplus)
#include <stdbool.h> /* C doesn't have booleans by default. */
#endif

#include <stddef.h>
#include <stdint.h>

/* Check if the compiler thinks we are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif

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

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

size_t		terminal_row;
size_t		terminal_column;
uint8_t		terminal_color;
uint16_t	*terminal_buffer;

uint8_t make_color(enum vga_color fg, enum vga_color bg) {
	return fg | bg << 4;
}

uint16_t make_vgaentry(char c, uint8_t color) {
	uint16_t c16 = c;
	uint16_t color16 = color;
	return c16 | color16 << 8;
}

size_t strlen(const char* str) {
	size_t ret = 0;
	while ( str[ret] != 0 )
		ret++;
	return ret;
}

void termInit() {
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = make_color(COLOR_LIGHT_GREY, COLOR_BLUE);
	terminal_buffer = (uint16_t*)0xB8000;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = make_vgaentry(' ', terminal_color);
		}
	}
}

void terminal_setcolor(uint8_t color) {
	terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = make_vgaentry(c, color);
}

void terminal_putchar(char c) {
	if(c == '\n') {
		terminal_column = 0;
		terminal_row++;
		return;
	}
	terminal_putentryat(c, terminal_color, terminal_column, terminal_row);

	if (++terminal_column == VGA_WIDTH) {
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT) {
			terminal_row = 0;
		}
	}
}

void termWriteString(const char* data) {
	size_t datalen = strlen(data);
	for (size_t i = 0; i < datalen; i++)
		terminal_putchar(data[i]);
}

struct ModeInfoBlock {
// Mandatory information for all VBE revisions
	uint16_t	ModeAttributes;
	uint8_t		WinAAttributes;
	uint8_t		WinBAttributes;
	uint16_t	WinGranularity;
	uint16_t	WinSize;
	uint16_t	WinASegment;
	uint16_t	WinBSegment;
	uint32_t	WinFuncPtr;
	uint16_t	BytesPerScanLine;

// Mandatory information for VBE 1.2 and above
	uint16_t	XResolution;
	uint16_t	YResolution;
	uint8_t		XCharSize;
	uint8_t		YCharSize;
	uint8_t		NumberOfPlanes;
	uint8_t		BitsPerPixel;
	uint8_t		NumberOfBanks;
	uint8_t		MemoryModel;
	uint8_t		BankSize;
	uint8_t		NumberOfImagePages;
	uint8_t		Reserved_page;

// Direct Color fields (required for direct/6 and YUV/7 memory models)
	uint8_t		RedMaskSize;
	uint8_t		RedFieldPosition;
	uint8_t		GreenMaskSize;
	uint8_t		GreenFieldPosition;
	uint8_t		BlueMaskSize;
	uint8_t		BlueFieldPosition;
	uint8_t		RsvdMaskSize;
	uint8_t		RsvdFieldPosition;
	uint8_t		DirectColorModeInfo;

// Mandatory information for VBE 2.0 and above
	uint32_t	PhysBasePtr;
	uint32_t	OffScreenMemOffset;
	uint16_t	OffScreenMemSize;

// Mandatory information for VBE 3.0 and above
	uint16_t	LinBytesPerScanLine;
	uint8_t		BnkNumberOfPages;
	uint8_t		LinNumberOfPages;
	uint8_t		LinRedMaskSize;
	uint8_t		LinRedFieldPosition;
	uint8_t		LinGreenMaskSize;
	uint8_t		LinGreenFieldPosition;
	uint8_t		LinBlueMaskSize;
	uint8_t		LinBlueFieldPosition;
	uint8_t		LinRsvdMaskSize;
	uint8_t		LinRsvdFieldPosition;
	uint32_t	MaxPixelClock;
	uint8_t		Reserved[189];
};

typedef union vbe_ptr {
	uint32_t		Ptr32;

//	void *Ptr;
//	void __far	*Ptr;
//	union {
//		uint16_t	Off;
//		uint16_t	Seg;
//	};

} vbe_ptr;

struct VbeInfoBlock {
	union {
		uint8_t		SigChr[4];
		uint32_t	Sig32;
	} VbeSignature;
	uint16_t	VbeVersion;
	vbe_ptr		OemString;
	uint8_t		Capabilities[4];
	uint16_t	VideoModePtr_Off;
	uint16_t	VideoModePtr_Seg;
	uint16_t	TotalMemory;
	uint16_t	OemSoftwareRev;
	uint32_t	*OemVendorName;
	vbe_ptr		OemProductName;
	vbe_ptr		OemProductRev;
	uint16_t	Reserved[111]; // used for dynamically generated mode list
	uint8_t		OemData[256];
};

struct MultibootInfo {
	uint32_t			flags;
	uint32_t			memLower;
	uint32_t			memUpper;
	uint32_t			bootDevice;
	uint32_t			cmdLine;
	uint32_t			modsCount;
	uint32_t			modsAddr;
	uint32_t			syms[4];
	uint32_t			mmapLength;
	uint32_t			mmapAddr;
	uint32_t			drivesLength;
	uint32_t			drivesAddr;
	uint32_t			configTable;
	uint32_t			bootLoaderName;
	uint32_t			apmTable;
	VbeInfoBlock	*vbeControlInfo;
	ModeInfoBlock	*vbeModeInfo;
	uint32_t			vbeInterfaceLength;
	uint32_t			vbeInterfaceSegment;
	uint32_t			vbeInterfaceOffset;
};

char hexChars[] = "0123456789ABCDEF";

void termWriteHex(uint32_t value) {
	for (size_t i = 0, j = (8-1)*4 ; i<8; ++i, j -= 4) {
		terminal_putchar(hexChars[(value >> j) & 0xf]);
	}
}

#if defined(__cplusplus)
extern "C"
#endif
void kernel_main(MultibootInfo *m, uint32_t checksum) {
	termInit();
	termWriteString("\n\nChecksum 0x");
	termWriteHex(checksum);
	termWriteString("\n\nMultibootInfo\n~~~~~~~~~~~~~~~~");
	termWriteString("\n     vbeControlInfo 0x");
	termWriteHex((uint32_t)m->vbeControlInfo);
	termWriteString("\n        vbeModeInfo 0x");
	termWriteHex((uint32_t)m->vbeModeInfo);
	termWriteString("\nvbeInterfaceSegment 0x");
	termWriteHex(m->vbeInterfaceSegment);
	termWriteString("\n vbeInterfaceOffset 0x");
	termWriteHex(m->vbeInterfaceOffset);
	termWriteString("\n vbeInterfaceLength 0x");
	termWriteHex(m->vbeInterfaceLength);

	VbeInfoBlock	*vi = m->vbeControlInfo;
	// ModeInfoBlock *mi = m->vbeModeInfo;

	termWriteString("\n vbeModeInfo->PhysBasePtrdd 0x");
	termWriteHex((uint32_t)vi->VideoModePtr_Seg);
}
