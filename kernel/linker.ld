OUTPUT_FORMAT("binary")
ENTRY("kernelmain")
SECTIONS {
	. = 0x10000;
	.kernelmain : { *(.kernelmain) }
	.text ALIGN(0x1000) : { *(.text) }
	.data ALIGN(0x1000) : { *(.data) *(.rodata) }
	.bss ALIGN(0x1000) : { *(.bss) }
}
/*	.rodata ALIGN(0x1000) : { *(.rodata) }
*/


