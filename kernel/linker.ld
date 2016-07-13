OUTPUT_FORMAT("binary")
ENTRY("kernelmain")
SECTIONS
{
	. = 0x10000;
	.text : { *(.text) }
	.data : { *(.data) } 
}
