
; #define VESA_SIGNATURE	'ASEV'
; #define VBE2_SIGNATURE	'2EBV'

; ==[ vbeInfoBlock ]===================================================================[ Struct ]==
;
; VBE 1.x returns 256 bytes
; VBE 2.x and 3.x returns 512 bytes
;
	struc vbeInfo
; -- VBE 1.x fields
.signature	ResD	1	; Has to be "VESA" or "VBE2"
.version	ResW	1
.oemString	ResD	1
.capabilities	ResD	1
.modeListPointer	ResD	1	; modeListPointer = 16bit pointer; modeListPointer + 2 = Segment
.totalMemory	ResW	1
; -- VBE 2.0+ fields only
.oemSoftwareRev	ResW	1
.oemVendorName	ResD	1
.oemProductName	ResD	1
.oemProductRev	ResD	1
.reserved	ResB	222
.oemData	ResB	256
.size	endstruc



; ==[ vbeModeInfo ]====================================================================[ Struct ]==
;
	struc vbeMode
.attributes	ResW	1	; attributes
.winA	ResB	1	; window_a_attributes
.winB	ResB	1	; window_b_attributes
.granularity	ResW	1	; window_granularity
.winSize	ResW	1	; window_size
.segmentA	ResW	1	; window_a_segment
.setmentB	ResW	1	; window_b_segment
.realFunctionPtr	ResD	1	; window_function - real mode pointer
.pitch	ResW	1	; bytes_per_row
; -- VBE 1.2 and above
.width	ResW	1	; width
.height	ResW	1	; height
.charWidth	ResB	1	; char_width
.charHeight	ResB	1	; char_height
.planes	ResB	1	; num_planes
.bpp	ResB	1	; bits_per_pixel
.banks	ResB	1	; num_banks
.memoryModel	ResB	1	; memory_model
.bankSize	ResB	1	; bank_size
.imagePages	ResB	1	; num_image_pages
.reserved0	ResB	1	; _reserved0
; -- direct color fields
.redMaskSize	ResB	1	; red_mask_size
.redMaskPos	ResB	1	; red_field_Pos
.greenMaskSize	ResB	1	; green_mask_size
.greenMaskPos	ResB	1	; green_field_Pos
.blueMaskSize	ResB	1	; blue_mask_size
.blueMaskPos	ResB	1	; blue_field_Pos
.reservedMaskSize	ResB	1	; reserved_mask_size
.reservedMaskPos	ResB	1	; reserved_field_Pos
.directColorModeInfo ResB	1	; direct_color_mode_info
; -- VBE 2.0 and above
.physicalBase	ResD	1	; physical_base
.reserved1	ResD	1	; _reserved1
.reserved2	ResW	1	; _reserved2
; -- VBE 3.0 and above
.linearBytesPerRow	ResW	1	; linear_bytes_per_row
.bankedNumImagePages ResB	1	; banked_num_image_pages
.linearNumImagePages ResB	1	; linear_num_image_pages
;
.linRedMaskSize	ResB	1	; linear_red_mask_size
.linRedMaskPos	ResB	1	; linear_red_field_Pos
.linGreenMaskSize	ResB	1	; linear_green_mask_size
.linGreenMaskPos	ResB	1	; linear_green_field_Pos
.linBlueMaskSize	ResB	1	; linear_blue_mask_size
.linBlueMaskPos	ResB	1	; linear_blue_field_Pos
.linReservedMaskSize ResB	1	; linear_reserved_mask_size
.linReservedMaskPos	ResB	1	; linear_reserved_field_Pos

.maxPixelClock	ResD	1	; max_pixel_clock in Hz

.reserved	ResB	189	; _reserved[189]
.size	endstruc
