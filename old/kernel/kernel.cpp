/* vim: tabstop=3 shiftwidth=4 noexpandtab:
**
**
**
*/

#if !defined(__cplusplus)
#include <stdbool.h>
#endif

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>


#include "multiboot2.h"


#include "types.h"

#include "psf.h"
#include "fontLat2Terminus16.h"

// #if defined(__linux__)
// #error "Wrong compiler"
// #endif
//
// #if !defined(__i386__)
// #error "x86-elf compiler please"
// #endif

#define defaultFrontColor	0xfff0a438
#define defaultBackColor	0xff0c1420

// struct pos {
// 	u8	x;
// 	u8	y;
// };

struct position {
	u8	x;
	u8	y;
};

struct tty {
	u8	width;
	u8	height;
	position	cursor;
};

u08	*font;
u16	*frameBuffer;

u16	charColor	= 0x0e;
u32	backColor	= defaultBackColor;
u32	frontColor	= defaultFrontColor;

PSF2	*psf;
tty	defaultTTY	= {};

void putchar(int c) {
	if(c == '\n') {
		defaultTTY.cursor.x = 0;
		++defaultTTY.cursor.y;
		return;
	}
	u16 character = charColor << 8 | c;

	frameBuffer[defaultTTY.cursor.x + defaultTTY.cursor.y * 80] = character;
	defaultTTY.cursor.x++;
}


static void printchar(char **str, int c) {
	if(str) {
		**str = c;
		++(*str);
	} else {
		putchar(c);
	}
}

#define PAD_RIGHT	1
#define PAD_ZERO	2

static int prints(char **out, const char *string, int width, int pad) {
	register int pc = 0, padchar = ' ';

	if(width > 0) {
		register int len = 0;
		register const char *ptr;
		for(ptr = string; *ptr; ++ptr) {
			++len;
		}
		if(len >= width) {
			width = 0;
		} else  {
			width -= len;
		}
		if(pad & PAD_ZERO) {
			padchar = '0';
		}
	}

	if(!(pad & PAD_RIGHT)) {
		for(; width > 0; --width) {
			printchar(out, padchar);
			++pc;
		}
	}

	for(; *string ; ++string) {
		printchar(out, *string);
		++pc;
	}

	for(; width > 0; --width) {
		printchar(out, padchar);
		++pc;
	}

	return pc;
}

#define BUFFER_LENGTH 20

static int printInteger(char **out, s64 i, int b, int sg, int width, int pad, int letbase) {
	char	buffer[BUFFER_LENGTH];
	char	*s;
	u32	t;
	u32	neg = 0;
	u32	chars = 0;
	u64	u = i;

	if(i == 0) {
		buffer[0] = '0';
		buffer[1] = '\0';
		return prints(out, buffer, width, pad);
	}

	if(sg && b == 10 && i < 0) {
		neg = 1;
		u = -i;
	}

	s = buffer + BUFFER_LENGTH - 1;
	*s = '\0';

	while(u) {
		t = u % b;
		if(t >= 10) {
			t += letbase - '0' - 10;
		}
		*--s = t + '0';
		u /= b;
	}

	if(neg) {
		if(width &&(pad & PAD_ZERO)) {
			printchar(out, '-');
			++chars;
			--width;
		} else {
			*--s = '-';
		}
	}

	return chars + prints(out, s, width, pad);
}

static int print(char **out, const char *format, va_list args) {
	register int width, pad;
	register int pc = 0;
	char scr[2];

	for(; *format != 0; ++format) {
		if(*format == '%') {
			++format;
			width = pad = 0;
			if(*format == '\0') break;
			if(*format == '%') goto out;
			if(*format == '-') {
				++format;
				pad = PAD_RIGHT;
			}
			while(*format == '0') {
				++format;
				pad |= PAD_ZERO;
			}
			for(; *format >= '0' && *format <= '9'; ++format) {
				width *= 10;
				width += *format - '0';
			}
			if(*format == 's') {
				register char *s =(char *)va_arg(args, int);
				pc += prints(out, s ? s : "(null)", width, pad);
				continue;
			}
			if(*format == 'd') {
				pc += printInteger(out, va_arg(args, s64), 10, 1, width, pad, 'a');
				continue;
			}
			if(*format == 'x') {
				pc += printInteger(out, va_arg(args, s64), 16, 0, width, pad, 'a');
				continue;
			}
			if(*format == 'X') {
				pc += printInteger(out, va_arg(args, s64), 16, 0, width, pad, 'A');
				continue;
			}
			if(*format == 'u') {
				pc += printInteger(out, va_arg(args, s64), 10, 0, width, pad, 'a');
				continue;
			}
			if(*format == 'c') {
				scr[0] =(char)va_arg(args, int);
				scr[1] = '\0';
				pc += prints(out, scr, width, pad);
				continue;
			}
		} else {
out:		printchar(out, *format);
			++pc;
		}
	}
	if(out)**out = '\0';
	va_end(args);
	return pc;
}

int kprintf(const char *format, ...) {
	va_list args;
	va_start(args, format);
	return print(0, format, args);
}

int sprintf(char *out, const char *format, ...) {
	va_list args;
	va_start(args, format);
	return print(&out, format, args);
}

//
// void	vtClear();
// void	vtWriteChar(char c);
// void	vtWriteChar(char c, pos p);
// void	vtWriteChar(char c, u8 color);
// void	vtWriteChar(char c, u8 color, pos p);
// void	vtWriteString(char *s);
// void	vtWriteString(char *s, pos p);
// void	vtWriteString(char *s, u8 color);
// void	vtWriteString(char *s, u8 color, pos p);
// void	vtWriteHex();


size_t strlen(const char *str) {
	size_t result = 0;
	while(*str++) {
 		result++;
	}
	return result;
}

void writeString(const char *stringBuffer, position *cur = &defaultTTY.cursor) {
	u16 fontRowData = 0;
	while(u8 character = *stringBuffer++) {
		if(character == '\n') {
			cur->y++;
			cur->x = 0;
		} else {
			for(u32 row = 0; row < psf->height; ++row) {
				if((fontRowData = font[(character * psf->charSize)+ row])) {
					for(u32 pixel = 0; pixel < psf->width; ++pixel) {
						if(fontRowData & 1 << pixel) {
							frameBuffer[(cur->x * 8)+((row +(cur->y * psf->height))* 1280)+(8 - pixel)] = frontColor;
						}
					};
				} else {
					continue;
				}
			};
			cur->x++;
		}
	}
}

void writeString(const char *stringBuffer, uint32_t color) {
	frontColor = color;
	writeString(stringBuffer, &defaultTTY.cursor);
}

/*
** dumpMemory(pointer, length, type)
**
** pointer	- raw linear pointer to where we should start reading information to write to console
** length	- how much of it should we write?
** type		- b,w,d,q,128bit,256bit,512bit...
*/

enum class Type {
	BYTE = 0,
	WORD,
	LONG,
	QUAD
};

// TODO(peter): Change type to an enum.
void dumpMemory(size_t *pointer, size_t length, Type type = Type::LONG) {

	switch(type) {
		case Type::BYTE: {
		} break;

		case Type::WORD: {
		} break;

		case Type::LONG: {
//			writeHex((uint32_t)pointer);
			writeString(": ");
			for(size_t i = 0; i < length; i += 4) {
//				writeHex((uint32_t)pointer[i]);
//				pointer += 4;
				writeString(" ");
			}
			writeString((char *)pointer);
		} break;

		case Type::QUAD: {
		} break;

	}

}

#if defined(__cplusplus)
extern "C"
#endif
void kernelMain(unsigned long m) {

	frameBuffer =(u16 *)0xb8000;
	defaultTTY.cursor = {0,0};

	multiboot_tag	*tag =(multiboot_tag *)(m + 8);
	u32				size = *(unsigned *)m;

// The easiest method for the timings is to use the PIT's mode 0.
//
// Write 0x30 to IO port 0x43 (select mode 0 for counter 0),
// then write your count value to 0x40, LSB first (e.g. write 0xA9 then 0x4 for a millisecond).
//
// To check if counter has finished, write 0xE2 to IO port 0x43, then read a status byte from
// port 0x40. If the 7th bit is set, then it has finished.



	kprintf("Size of Multiboot structure: %d\n", size);

	for(; tag->type != MULTIBOOT_TAG_TYPE_END; tag =(multiboot_tag *)((u8 *)tag +((tag->size + 7)& ~ 7))) {
		switch(tag->type) {
			case MULTIBOOT_TAG_TYPE_CMDLINE:
				break;

			case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
				break;

			case MULTIBOOT_TAG_TYPE_MODULE:
				break;

			case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO: {
					multiboot_tag_basic_meminfo *meminfo = (multiboot_tag_basic_meminfo *)tag;
					kprintf("\nMemInfo - Lower: %dkb(max 640kb)- Upper %dkb(from 1mb and up)\n", meminfo->mem_lower, meminfo->mem_upper);
				} break;

			case MULTIBOOT_TAG_TYPE_BOOTDEV:
				break;

			case MULTIBOOT_TAG_TYPE_MMAP: {
					multiboot_tag_mmap *mmap =(multiboot_tag_mmap *)tag;

					kprintf("\nMMAP (%d entries).\n", (mmap->size / mmap->entry_size));
					for(unsigned int i = 0; i < (mmap->size/mmap->entry_size); ++i) {
						if(mmap->entries[i].addr < 0x100000) continue;
						kprintf(" Offset: %016x Length: %016x Type: ",(u64)mmap->entries[i].addr, (u64)mmap->entries[i].len);

						switch(mmap->entries[i].type) {
							case 1:
								kprintf("Available RAM.\n");
								break;
							case 3:
								kprintf("Usable memory holding ACPI information.\n");
								break;
							case 4:
								kprintf("Reserved memory, has to be preserved on hibernation.\n");
								break;
							default:
								kprintf("Reserved memory/area.\n");
						}
					}

				} break;

			case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:
				multiboot_tag_framebuffer *tagfb =(multiboot_tag_framebuffer *)tag;

				kprintf("\nFramebuffer\n - Address: %016x\n",(u64)tagfb->common.framebuffer_addr);

				kprintf(" - Type: ");

				switch(tagfb->common.framebuffer_type) {
					case MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED:
						kprintf("Indexed\n");
						break;

					case MULTIBOOT_FRAMEBUFFER_TYPE_RGB:
						kprintf("RGB\n");
						break;

					case MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT:
						kprintf("EGA TEXT\n");
						break;
//					case default:
				}
				switch(tagfb->common.framebuffer_bpp) {
					case 8:
					case 15:
					case 16:
					case 24:
					case 32:
						break;
				}
		}
	}


	while(1) {}

//	termInit();


//	uint32_t *ptr =(uint32_t *)0x40000;		// so we can write stuff and read out from qemu -monitor stdio
//	ptr[0] = strlen(stringBuffer);

//	modeInfo		= m->vbeModeInfoBlock;
	frameBuffer	= 0;//(size_t *)modeInfo->PhysBasePtr;
	psf			=(PSF2 *)&fontLat2Terminus16;
	font			=(uint8_t *)(psf)+ psf->headerSize;

	for(size_t i = 0; i < 1280*720; ++i) {	// clear screen
		frameBuffer[i] = backColor;
	}

	defaultTTY.cursor.x		= 0;
	defaultTTY.cursor.y		= 0;
	frontColor	= 0xfff0a438;

	// writeString("    KAOS v0.0.0 - Created by Mindkiller Systems.\n"
   //  				"    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
//	writeHex(uint32_t(&modeInfo));

//	position pos = {30, 30};
//	writeString("This is a test\n", &pos);;
//	writeString("This is a test 0x", 0xff474849);
//	writeHex(0x4748494a, defaultFrontColor);
	writeString("\n");

//	dumpMemory((size_t *)(size_t)((m->vbeInfoBlock->OemVendorNamePtr_Seg << 4)+ m->vbeInfoBlock->OemVendorNamePtr_Off), 0x8);

	while(1) {
	// for(size_t i = 0; i < 1280*720; ++i) {	// clear screen
	// 	frameBuffer[i] = backColor;
	}

}
