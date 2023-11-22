OUTPUT_FORMAT("elf32-powerpc", "elf32-powerpc", "elf32-powerpc");
OUTPUT_ARCH(powerpc:common);
ENTRY(wwfc_payload_entry)

MEMORY
{
	code (rwx) : ORIGIN = 0x00000000, LENGTH = 0x01800000
}

SECTIONS
{
	.text : {
		KEEP (*(wwfc_header))

		_G_GOTStart = .;
		*(.got2)
		_G_GOTEnd = .;

		_G_FixupStart = .;
		KEEP(*(.fixup))
		_G_FixupEnd = .;

		_G_WWFCPatchStart = .;
		KEEP(*(.wwfc_patch))
		_G_WWFCPatchEnd = .;

		*(.text*)
		*(.insert*)
		*(.rodata*)
		*(.data*)
		*(.bss*)

		__SDATA_START__ = .;
		*(.sdata*)
		__SDATA_END__ = .;

		__SDATA2_START__ = .;
		*(.sdata2*)
		__SDATA2_END__ = .;

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
