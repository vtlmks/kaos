/*OUTPUT_FORMAT("binary")*/
/*ENTRY(loader)*/
/*
SECTIONS
{


	.text 0x100000 :
	{
	code = .; _code = .;

	*(.text)
	. = ALIGN(4096);
	}

	.data :
	{
	data = .; 
	*(.data)
	. = ALIGN(4096);
	}

	end = .; _end =.; __end =.;
}
*/
ENTRY("kernelmain")
SECTIONS
{
	. = 0x10000;
	.text : { *(.text) }
	.data : { *(.data) } 
}