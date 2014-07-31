OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start32)
SECTIONS
{
	. = 0x100;

	.bootstrap32 :
	{
		header.o (.data)
		_bootstrap32 = .;
		start32.o (.text)
		start32.o (.data)
	}

	. = 0x1000;
	.text :
	{
		start64.o (.text)
		boot_kmain.o (.text)
		*(.rodata*)
		. = ALIGN(0x1000);
	}

	.data :
	{
		*(.data)
	}

	. = ALIGN(0x1000);
	.bss :
	{
		*(.bss)
	}
	. = ALIGN(0x1000);
	_kern_end = .;

	. = 0x90000;
	_boot_hat = .;
}
