OUTPUT_FORMAT("elf32-powerpc", "elf32-powerpc", "elf32-powerpc");
OUTPUT_ARCH(powerpc:common);
ENTRY(wwfcStage1Entry)

MEMORY
{
	code (rwx) : ORIGIN = 0x00000000, LENGTH = 0x01800000
}

SECTIONS
{
	.text : {
		KEEP (*(.start))
		. = ALIGN(4);
		*(.text*)
		*(.rodata*)
		*(.data*)
		*(.bss*)
		*(.sdata*)
		*(.sbss*)
		wwfc_stage1_end = .;
	} > code

	/DISCARD/ : {
		*(.comment*)
		*(.eh_frame*)
		*(.eh_frame_hdr*)
	}
}
