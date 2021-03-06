
#include <stdarg.h>
#include <stdint.h>
#include <types.h>

//#include <fontLat2Terminus16.h>
#include <moSoul.h>
#include <loaderinfo.h>
#include <psf.h>

#define	defaultFrontColor	0xfff0a438
#define	defaultBackColor	0xff0c1420

// TODO(peter): Make it possible to use different colors
//
//	Black
//	blue, green, cyan, red, magenta, brown
//	light grey, dark grey
//	lightblue, -green, -cyan, -red, -magenta, -brown
//	white

struct Pos {
	u8	x;
	u8	y;
};

struct Screen {
	u16 width;
	u16 height;
};

struct TTY {
	u16	charWidth;
	u16	charHeight;
	Pos	curPos;
};

PSF2	*psf;
u8		*font;
u32		*frameBuffer;

Screen	defaultScreen;
TTY		defaultTTY;

void scrollOneUp() {
	u16 rows = defaultScreen.height - psf->height;

	u64 fill = defaultBackColor;
	fill = fill << 32 | defaultBackColor;

	u64 *fb = (u64*)frameBuffer;
	u64 *fb2 = (u64*)frameBuffer + (defaultScreen.width * psf->height / 2);

	for(u32 i = 0; i < u32((defaultScreen.width * rows) / 2); ++i) {
		*fb++ = *fb2++;
	}

	// no need to reinitialize fb as the pointer should already be set to correct position here		was before:	fb = (u64*)frameBuffer + ((rows * defaultScreen.width) / 2);
	for(u32 i = 0; i < u32((defaultScreen.width * psf->height) / 2); ++i) {
		*fb++ = fill;
	}
}

void clearScreen() {
	u64 *fb = (u64*)frameBuffer;

	u64 fill = defaultBackColor;
	fill = fill << 32 | defaultBackColor;

	for(u32 i = 0; i < u32((defaultScreen.width * defaultScreen.height) / 2); ++i) {
		*fb++ = fill;
	}
}

/*
 * Initialize TTY
 */
void setupTTY(LoaderInfo *info) {
	psf						= (PSF2 *)&moSoul;
	font					= (u8 *)(psf) + psf->headerSize;
	frameBuffer				= info->vesaPhysBasePtr;

	defaultScreen.width		= info->vesaPixelWidth;
	defaultScreen.height	= info->vesaPixelHeight;

	defaultTTY.charWidth	= info->vesaPixelWidth / psf->width;
	defaultTTY.charHeight	= info->vesaPixelHeight / psf->height;
	defaultTTY.curPos.x		= 0;
	defaultTTY.curPos.y		= 0;

	clearScreen();
}


void writeChar(int character, Pos *cur) {
	// NOTE(peter): add support for character sets with characters wider than 8 pixels, this was overlooked first time through.

	u16 fontRowData;
	for(u8 row = 0; row < psf->height; ++row) {
		fontRowData = font[(character * psf->charSize) + 0 + row];
		if(fontRowData) {
			for(u8 pixel = 0; pixel < psf->width; ++pixel) {
				if(fontRowData & (1 << pixel)) {
					frameBuffer[(cur->x * psf->width) + ((row + (cur->y * psf->height)) * defaultScreen.width) + (psf->width - pixel)] = defaultFrontColor;
				}
			}
		}
	}
}

void advanceCursorX() {
	++defaultTTY.curPos.x;
	if(defaultTTY.curPos.x >= defaultTTY.charWidth) {
		defaultTTY.curPos.x = 0;
		++defaultTTY.curPos.y;
	}
	if(defaultTTY.curPos.y >= defaultTTY.charHeight) {
		--defaultTTY.curPos.y;
		scrollOneUp();
	}
}

void newLine() {
	defaultTTY.curPos.x = 0;
	++defaultTTY.curPos.y;
	if(defaultTTY.curPos.y >= defaultTTY.charHeight) {
		--defaultTTY.curPos.y;
		scrollOneUp();
	}
}

void printchar(char **str, int c) {
	if(str) {
		**str = c;
		++(*str);
	} else {
		if(c == '\n') {
			newLine();
		} else {
			writeChar(c, &defaultTTY.curPos);
			advanceCursorX();
		}
	}
}

#define PAD_RIGHT	1
#define PAD_ZERO	2

int prints(char **out, const char *string, int width, int pad) {
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

int printInteger(char **out, s64 i, int b, int sg, int width, int pad, int letbase) {
	char	buffer[BUFFER_LENGTH];
	char	*s;
	u32		t;
	u32		neg = 0;
	u32		chars = 0;
	u64		u = i;

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

int print(char **out, const char *formatString, va_list args) {
	int width, pad;
	int pc = 0;
	char scr[2];

	for(; *formatString != 0; ++formatString) {
		if(*formatString == '%') {
			++formatString;
			width = pad = 0;
			if(*formatString == '\0') break;
			if(*formatString == '%') goto next;
			if(*formatString == '-') {
				++formatString;
				pad = PAD_RIGHT;
			}
			while(*formatString == '0') {
				++formatString;
				pad |= PAD_ZERO;
			}
			for(; *formatString >= '0' && *formatString <= '9'; ++formatString) {
				width *= 10;
				width += *formatString - '0';
			}
			if(*formatString == 's') {
				char *s =(char *)va_arg(args, int);
				pc += prints(out, s ? s : "(null)", width, pad);
				continue;
			}
			if(*formatString == 'd') {
				pc += printInteger(out, va_arg(args, s64), 10, 1, width, pad, 'a');
				continue;
			}
			if(*formatString == 'x') {
				pc += printInteger(out, va_arg(args, s64), 16, 0, width, pad, 'a');
				continue;
			}
			if(*formatString == 'X') {
				pc += printInteger(out, va_arg(args, s64), 16, 0, width, pad, 'A');
				continue;
			}
			if(*formatString == 'u') {
				pc += printInteger(out, va_arg(args, s64), 10, 0, width, pad, 'a');
				continue;
			}
			if(*formatString == 'c') {
				scr[0] =(char)va_arg(args, int);
				scr[1] = '\0';
				pc += prints(out, scr, width, pad);
				continue;
			}
		} else {
next:		printchar(out, *formatString);
			++pc;
		}
	}
	if(out)**out = '\0';
	va_end(args);
	return pc;
}

int kprintf(const char *formatString, ...) {
	va_list args;
	va_start(args, formatString);
	return print(0, formatString, args);
}

int sprintf(char *out, const char *formatString, ...) {
	va_list args;
	va_start(args, formatString);
	return print(&out, formatString, args);
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

// size is 0, 1, 2, 3 - 8, 16, 32, 64
void dumpMem(u64 offset, u16 rows, u8 size = 3) {

	for(; rows > 0; --rows) {
	}

	switch(size) {
		case 0: {
			u32	a = *(u32*)offset + 0;
			u32 b = *(u32*)offset + 1;
			u32	c = *(u32*)offset + 2;
			u32	d = *(u32*)offset + 3;
			u8	ascii[16];

			kprintf("%x: %x %x %x %x  %8s\n", a, b, c, d, ascii);
		} break;

		case 1: {
		} break;

		case 2: {
		} break;

		case 3: {
		} break;
	}
}
