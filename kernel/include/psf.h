#ifndef PSF_H
#define PSF_H

#define PSF2_MAGIC0     0x72
#define PSF2_MAGIC1     0xb5
#define PSF2_MAGIC2     0x4a
#define PSF2_MAGIC3     0x86

/* bits used in flags */
#define PSF2_HAS_UNICODE_TABLE 0x01

/* max version recognized so far */
#define PSF2_MAXVERSION 0

/* UTF8 separators */
#define PSF2_SEPARATOR  0xFF
#define PSF2_STARTSEQ   0xFE

struct PSF2 {
	uint8_t	magic[4];
	uint32_t	version;
	uint32_t	headerSize;		/* offset of bitmaps in file */
	uint32_t	flags;
	uint32_t	length;			/* number of glyphs */
	uint32_t	charSize;		/* number of bytes for each character */
	uint32_t	height;
	uint32_t	width;			/* max dimensions of glyphs */
	/* charsize = height * ((width + 7) / 8) */
};


#endif
