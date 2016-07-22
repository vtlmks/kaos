OUTPUT_FORMAT("binary")
ENTRY("kernelmain")
SECTIONS {
	. = 0x10000;
	.kernelmain : {
		*(.kernelmain)
	}

	.text : {
		*(.text)
	}

	.rodata : {
		*(.rodata)
	}

	.data : {
		*(.data)
	}

	.bss : {
		*(.bss)
	}
}
