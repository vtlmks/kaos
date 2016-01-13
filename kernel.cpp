// vim:set tabstop=2:
#if !defined(__cplusplus)
#include <stdbool.h> /* C doesn't have booleans by default. */
#endif

#include <stddef.h>
#include <stdint.h>

#include "psf.h"
#include "fontLat2Terminus16.h"

#if defined(__linux__)
#error "Wrong compiler"
#endif

#if !defined(__i386__)
#error "x86-elf compiler please"
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

struct VbeModeInfoBlock {
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

struct VbeInfoBlock {
	uint8_t	VbeSignature[4];
	uint16_t	VbeVersion;
	uint16_t	OemStringPtr_Off;
	uint16_t	OemStringPtr_Seg;
	uint8_t		Capabilities[4];
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
	VbeInfoBlock		*vbeInfoBlock;					// control_info
	VbeModeInfoBlock	*vbeModeInfoBlock;				// mode_info
	uint16_t			vbeMode;
	uint16_t			vbeInterfaceSegment;
	uint16_t			vbeInterfaceOffset;
	uint16_t			vbeInterfaceLength;
};

void termWriteHex(uint32_t value) {
	static const char hexChars[] = "0123456789ABCDEF";
	char temp[8];
	for (size_t i = 0, j = (8-1)*4 ; i<8; ++i, j -= 4) {
		temp[i] = hexChars[(value >> j) & 0xf];
	}
	termWriteString(temp);
}

#if defined(__cplusplus)
extern "C"
#endif
void kernel_main(MultibootInfo *m, uint32_t checksum) {
	termInit();
	termWriteHex((uint32_t)checksum);

	VbeModeInfoBlock	*mi						= m->vbeModeInfoBlock;
	uint32_t					*frameBuffer	= (uint32_t *)mi->PhysBasePtr;
	PSF2							*psf					= (PSF2 *)&fontLat2Terminus16;
	uint8_t						*font					= (uint8_t *)(psf) + psf->headerSize;

	struct vec2 {
		uint8_t	x;
		uint8_t	y;
	};
	vec2 cursor = {0,0};
	uint32_t	fontColor = 0xffaaaaaa;
	uint32_t	backColor = 0xff0e1723;

	const char *stringBuffer =	"KAOS v0.0.0 - Created by Mindkiller Systems.\n"
															"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n"
															"A multilevel feedback queue should give preference to short jobs, I/O bound processes,\n"
															"separate processes into categories based on their need for the processor <wikipedia>\n"
 															"  - How often the task has to be preempted can be used to decide processor bound processes, after a while the process will migrate to a lower-priority queue.\n"
 															"  - If the process yields itself due to waiting for I/O decides if and how I/O bound a process is.\n"
 															"  - For short jobs, no real idea, but we could give a larger quanta from the start of a new process and lower the quanta to the standard level shortly after.\n"
 															"  - A process that waits for too long in a lower-priority queue may be moved into a higher-priority queue. - This is to prevent starvation of lower-priority processes.\n"
 															"  - Processes that yields to wait for I/O will gain higher priority so that they are more responsive and finishes faster (in theory)\n\n"
 															"Highest priority (low quanta)\n"
  														"  - system processes\n"
  														"  - interactive processes\n"
  														"  - interactive editing processes\n"
  														"  - batch processes\n"
  														"  - other processes (sending data to microsoft servers or somesuch)\n"
 															"Lowest priority (high quanta)\n\n"
 															"In general, a multilevel feedback queue scheduler is defined by the following parameters:\n\n"
 															"  - The number of queues.\n"
  														"  - The scheduling algorithm for each queue which can be different from FIFO.\n"
  														"  - The method used to determine when to promote a process to a higher priority queue.\n"
  														"  - The method used to determine when to demote a process to a lower priority queue.\n"
  														"  - The method used to determine which queue a process will enter when that process needs service.\n"

															"\0";	// HAS to end with null..

	for(size_t i = 0; i < 1280*720; ++i) {	// clear screen
		frameBuffer[i] = backColor;
	}

	while(*stringBuffer) {
		uint8_t character = *stringBuffer++;

		if(character == '\n') {
			cursor.y++;
			cursor.x = 0;
		} else {
			for(uint32_t row = 0; row < psf->height; ++row) { //psf->height; ++y) {
				uint16_t fontRowData = font[(character * psf->charSize) + row];
				for(uint32_t pixel = 0; pixel < psf->width; ++pixel) {
					if (fontRowData & 1 << pixel) {
						frameBuffer[cursor.x*8 + ((row + (cursor.y*psf->height)) * 1280) + (8 - pixel)] = fontColor;
					}
				};
			};
			cursor.x++;
		}
	}

	while(1) {
	}

//	termWriteHex((uint32_t)mi->PhysBasePtr);

}
