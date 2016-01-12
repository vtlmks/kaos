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

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;

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

void terminal_initialize() {
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = make_color(COLOR_LIGHT_GREY, COLOR_BLUE);
	terminal_buffer = (uint16_t*) 0xB8000;
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

void terminal_writestring(const char* data) {
	size_t datalen = strlen(data);
	for (size_t i = 0; i < datalen; i++)
		terminal_putchar(data[i]);
}

typedef struct MultibootInfo {
	uint32_t	flags;
	uint32_t	memLower;
	uint32_t	memUpper;
	uint32_t	bootDevice;
	uint32_t	cmdLine;
	uint32_t	modsCount;
	uint32_t	modsAddr;
	uint32_t	syms[4];
	uint32_t	mmapLength;
	uint32_t	mmapAddr;
	uint32_t	drivesLength;
	uint32_t	drivesAddr;
	uint32_t	configTable;
	uint32_t	bootLoaderName;
	uint32_t	apmTable;
	uint32_t	vbeControlInfo;
	uint32_t	vbeModeInfo;
	uint32_t	vbeInterfaceSegment;
	uint32_t	vbeInterfaceOffset;
	uint32_t	vbeInterfaceLength;
} MultibootInfo;

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
	terminal_initialize();
	terminal_writestring("MultibootInfo\n~~~~~~~~~~~~~~~~");
	terminal_writestring("\n     vbeControlInfo 0x");
	termWriteHex(m->vbeControlInfo);
	terminal_writestring("\n        vbeModeInfo 0x");
	termWriteHex(m->vbeModeInfo);
	terminal_writestring("\nvbeInterfaceSegment 0x");
	termWriteHex(m->vbeInterfaceSegment);
	terminal_writestring("\n vbeInterfaceOffset 0x");
	termWriteHex(m->vbeInterfaceOffset);
	terminal_writestring("\n vbeInterfaceLength 0x");
	termWriteHex(m->vbeInterfaceLength);


	terminal_writestring("\n\n test 0x");
	termWriteHex(0x12345678);


}

