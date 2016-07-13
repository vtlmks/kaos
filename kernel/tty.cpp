
#include <stdarg.h>
#include <stdint.h>
#include <loaderinfo.h>

#include "include\types.h"
#include "fontLat2Terminus16.h"
#include "psf.h"


#define	defaultFrontColor	0xfff0a438
#define	defaultBackColor	0xff0c1420

struct Pos {
	u8	x;
	u8	y;
};

struct Screen {
	u16 width;
	u16 height;
};

struct TTY {
	u16	widht;
	u16	height;
	u16	charWidth;
	u16	charHeight;
	Pos	cursorPos;
};

PSF2	*psf;
u8		*font;
u32	*frameBuffer;

u32	backColor;
u32	frontColor;

u16	charColor;
u16	screenWidth;
u16	screenHeight;
Pos	cursor;

TTY	defaultTTY;

void putchar(int c) {
	if(c == '\n') {
		defaultTTY.cursorPos.x = 0;
		++defaultTTY.cursorPos.y;
		return;
	}
	u16 character = charColor << 8 | c;

	frameBuffer[defaultTTY.cursorPos.x + defaultTTY.cursorPos.y * 80] = character;
	defaultTTY.cursorPos.x++;
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
	int pc = 0, padchar = ' ';

	if(width > 0) {
		int len = 0;
		const char *ptr;
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
	int width, pad;
	int pc = 0;
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
				char *s =(char *)va_arg(args, int);
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

/*
 * Check null terminated string length
 */
u32 strlen(const char *str) {
	u32 result = 0;
	while(*str++) {
		result++;
	}
	return result;
}

void writeString(const char *stringBuffer, Pos *cur = &cursor) {
	u16 fontRowData = 0;
	while(u8 character = *stringBuffer++) {
		if(character == '\n') {
			++cur->y;
			cur->x = 0;
		} else {
			for(u32 row = 0; row < psf->height; ++row) {
				if((fontRowData = font[(character * psf->charSize) + row])) {
					for(u32 pixel = 0; pixel < psf->width; ++pixel) {
						if(fontRowData & 1 << pixel) {
							frameBuffer[(cur->x * psf->width) + ((row + (cur->y * psf->height)) * 1280) + (psf->width - pixel)] = defaultFrontColor;
						}
					}
				} else {
					continue;
				}
			}
			++cur->x;
		}
	}
}


void writeString(const char *stringBuffer, u32 color) {
	frontColor = color;
	writeString(stringBuffer, &cursor);
}

void ttyInit(LoaderInfo *info) {
	char buffer[200];

	cursor		= {0, 0};
	psf			= (PSF2 *)&fontLat2Terminus16;
	font			= (u8 *)(psf) + psf->headerSize;

	frameBuffer	= info->vesaPhysBasePtr;	// TODO: Fix this.
	backColor	= defaultBackColor;
	frontColor	= defaultFrontColor;

	memInfo *e820Mem	= info->memInfoPtr;

	for(u32 i = 0; i < 1280 * 720; ++i) {	// clear screen
		frameBuffer[i] = defaultBackColor;
	}

	writeString("KAOS v0.1.0 - Created by Mindkiller Systems in 1916.\n");

	sprintf(buffer, "\nScreen mode %dx%d @ %d bits per pixel; %d bytes per row.\n", info->vesaPixelWidth, info->vesaPixelHeight, info->vesaPixelDepth, info->vesaBytesPerRow);
	writeString(buffer);

	sprintf(buffer, "\nMemlist\n~~~~~~~\n");
	writeString(buffer);

	for(u8 i = 0; i < info->memInfoCount; ++i) {
		sprintf(buffer, " From: 0x%016x  Size: 0x%016x Type: %1d\n", e820Mem[i].from, e820Mem[i].length, e820Mem[i].flag);
		writeString(buffer);
	}
}

