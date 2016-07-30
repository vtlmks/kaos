#pragma once

#include <types.h>
#include <e820.h>

struct LoaderInfo {
	e820entry	*e820Ptr;
	u32			*vesaPhysBasePtr;
	u16			vesaBytesPerRow;
	u16			vesaPixelWidth;
	u16			vesaPixelHeight;
	u8			e820Count;
	u8			vesaPixelDepth;
} __attribute__ ((__packed__));

//struct screen_info {                                                                 
//	u8		orig_x;                   /* 0x00 */            
//	u8		orig_y;                   /* 0x01 */            
//	u16		ext_mem_k;                /* 0x02 */            
//	u16		orig_video_page;          /* 0x04 */            
//	u8		orig_video_mode;          /* 0x06 */            
//	u8		orig_video_cols;          /* 0x07 */            
//	u8		flags;                    /* 0x08 */            
//	u8		unused2;                  /* 0x09 */            
//	u16		orig_video_ega_bx;        /* 0x0a */            
//	u16		unused3;                  /* 0x0c */            
//	u8		orig_video_lines;         /* 0x0e */            
//	u8		orig_video_isVGA;         /* 0x0f */            
//	u16		orig_video_points;        /* 0x10 */            
//
//	/* VESA graphic mode -- linear frame buffer */
//	u16		lfb_width;                /* 0x12 */            
//	u16		lfb_height;               /* 0x14 */            
//	u16		lfb_depth;                /* 0x16 */            
//	u32		lfb_base;                 /* 0x18 */            
//	u32		lfb_size;                 /* 0x1c */            
//	u16		cl_magic;                 /* 0x20 */
//	u16		cl_offset;                /* 0x22 */
//	u16		lfb_linelength;           /* 0x24 */            
//	u8		red_size;                 /* 0x26 */            
//	u8		red_pos;                  /* 0x27 */            
//	u8		green_size;               /* 0x28 */            
//	u8		green_pos;                /* 0x29 */            
//	u8		blue_size;                /* 0x2a */            
//	u8		blue_pos;                 /* 0x2b */            
//	u8		rsvd_size;                /* 0x2c */            
//	u8		rsvd_pos;                 /* 0x2d */            
//	u16		vesapm_seg;               /* 0x2e */            
//	u16		vesapm_off;               /* 0x30 */            
//	u16		pages;                    /* 0x32 */            
//	u16		vesa_attributes;          /* 0x34 */            
//	u32		capabilities;             /* 0x36 */            
//	u8		_reserved[6];             /* 0x3a */            
//} __attribute__((packed));                            
//
///* The so-called "zeropage" */
//struct boot_params {
//	screen_info		screen_info;                              /* 0x000 */
//	apm_bios_info	apm_bios_info;                            /* 0x040 */
//	u8				_pad2[4];                                 /* 0x054 */
//	u64				tboot_addr;                               /* 0x058 */
//	ist_info		ist_info;                                 /* 0x060 */
//	u8				_pad3[16];                                /* 0x070 */
//	u8				hd0_info[16];     /* obsolete! */         /* 0x080 */
//	u8				hd1_info[16];     /* obsolete! */         /* 0x090 */
//	sys_desc_table	sys_desc_table;                           /* 0x0a0 */
//	u8				_pad4[144];                               /* 0x0b0 */
//	edid_info		edid_info;                                /* 0x140 */
//	efi_info		efi_info;                                 /* 0x1c0 */
//	u32				alt_mem_k;                                /* 0x1e0 */
//	u32				scratch;          /* Scratch field! */    /* 0x1e4 */
//	u8				e820_entries;                             /* 0x1e8 */
//	u8				eddbuf_entries;                           /* 0x1e9 */
//	u8				edd_mbr_sig_buf_entries;                  /* 0x1ea */
//	u8				_pad6[6];                                 /* 0x1eb */
//	setup_header	hdr;                  /* setup header */  /* 0x1f1 */
//	u8				_pad7[0x290-0x1f1-sizeof(struct setup_header)];
//	u32				edd_mbr_sig_buffer[EDD_MBR_SIG_MAX];      /* 0x290 */
//	e820entry		e820_map[E820MAX];                        /* 0x2d0 */
//	u8				_pad8[48];                                /* 0xcd0 */
//	edd_info		eddbuf[EDDMAXNR];                         /* 0xd00 */
//	u8				_pad9[276];                               /* 0xeec */
//} __attribute__((packed));
//
//
