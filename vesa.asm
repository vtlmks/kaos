
; #define VESA_SIGNATURE	'ASEV'
; #define VBE2_SIGNATURE	'2EBV'

; ==[ vbeInfoBlock ]===================================================================[ Struct ]==
;
; VBE 1.x returns 256 bytes
; VBE 2.x and 3.x returns 512 bytes
	struc farPtr
.offset	ResW	1
.segment	ResW	1
.size	endstruc

	struc vbeInfo
; -- VBE 1.x fields
.signature	ResD	1	; Has to be "VESA" or "VBE2"
.version	ResW	1
.vendorString	ResD	1
.capabilities	ResD	1
.modeListPtr	ResB	farPtr.size	; modeListPointer = 16bit pointer; modeListPointer + 2 = Segment
.totalMemory	ResW	1
; -- VBE 2.0+ fields only
.vendorSoftwareRev	ResW	1
.VendorName	ResD	1
.vendorProductName	ResD	1
.vendorProductRev	ResD	1
.reserved	ResB	222
.vendorData	ResB	256
.size	endstruc

; ==[ vbeModeInfo ]====================================================================[ Struct ]==
	struc vbeModeInfo
.attributes	ResW	1	; attributes
.winAttributes	ResB	2	; window_attributes
.winGranularity	ResW	1	; window_granularity
.winSize	ResW	1	; window_size
.winSeg	ResW	2	; window_segments
.realmodePtr	ResB	farPtr.size	; window_function - real mode pointer
.pitch	ResW	1	; bytes_per_row
; -- VBE 1.2 and above
.screenWidth	ResW	1	; width
.screenHeight	ResW	1	; height
.charWidth	ResB	1	; char_width
.charHeight	ResB	1	; char_height
.planes	ResB	1	; num_planes
.bpp	ResB	1	; bits_per_pixel
.banks	ResB	1	; num_banks
.memoryLayout	ResB	1	; memory_model
.bankSize	ResB	1	; bank_size
.imagePages	ResB	1	; num_image_pages
.pageFunction	ResB	1	; page_function
; -- direct color fields
.rMask	ResB	1	; red_mask_size
.rPos	ResB	1	; red_field_Pos
.gMask	ResB	1	; green_mask_size
.gPos	ResB	1	; green_field_Pos
.bMask	ResB	1	; blue_mask_size
.bPos	ResB	1	; blue_field_Pos
.resvMask	ResB	1	; reserved_mask_size
.resvPos	ResB	1	; reserved_field_Pos
.dcmInfo	ResB	1	; direct_color_mode_info
; -- VBE 2.0 and above
.lfbPtr	ResD	1	; physical_base address
.offscreenPtr	ResD	1	; offscreen memory address
.offscreenSize	ResW	1	;
; -- VBE 3.0 and above
.linearBytesPerRow	ResW	1	; linear_bytes_per_row
.bankedNumImagePages	ResB	1	; banked_num_image_pages
.linearNumImagePages	ResB	1	; linear_num_image_pages
;
.linRMask	ResB	1	; linear_red_mask_size
.linRPos	ResB	1	; linear_red_field_Pos
.linGMask	ResB	1	; linear_green_mask_size
.linGPos	ResB	1	; linear_green_field_Pos
.linBMask	ResB	1	; linear_blue_mask_size
.linBPos	ResB	1	; linear_blue_field_Pos
.linResvMask	ResB	1	; linear_reserved_mask_size
.linResvPos	ResB	1	; linear_reserved_field_Pos
;
.maxPixelClock	ResD	1	; max_pixel_clock in Hz
.reserved	ResB	189	; _reserved[189]
.size	endstruc
