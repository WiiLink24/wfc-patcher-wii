OUTPUT_FORMAT("elf32-powerpc", "elf32-powerpc", "elf32-powerpc");
OUTPUT_ARCH(powerpc:common);
ENTRY(wwfc_payload_entry)

MEMORY
{
	code (rwx) : ORIGIN = 0x00000000, LENGTH = 0x00020000
}

SECTIONS
{
	.text : {
		KEEP (*(wwfc_header))

		. = ALIGN(4);
		_G_GOTStart = .;
		*(.got2)
		_G_GOTEnd = .;

		. = ALIGN(4);
		_G_FixupStart = .;
		KEEP(*(.fixup))
		_G_FixupEnd = .;

		. = ALIGN(4);
		_G_WWFCPatchStart = .;
		KEEP(*(.wwfc_patch))
		_G_WWFCPatchEnd = .;

		. = ALIGN(4);
		__CTORS_START__ = .;
		KEEP(*(.ctors*))
		__CTORS_END__ = .;

		*(.text*)
		*(.insert*)
		*(.rodata*)
		*(.data*)

		__SDATA_START__ = .;
		*(.sdata*)
		__SDATA_END__ = .;

		__SDATA2_START__ = .;
		*(.sdata2*)
		__SDATA2_END__ = .;

		*(.bss*)

		__SBSS_START__ = .;
		*(.sbss*)
		__SBSS_END__ = .;

		__SBSS2_START__ = .;
		*(.sbss2*)
		__SBSS2_END__ = .;

		_G_End = .;
	} > code

	/DISCARD/ : {
		*(.comment*)
		*(.eh_frame*)
		*(.eh_frame_hdr*)
	}
}
