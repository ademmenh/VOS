[bits 32]

section .rodata
global EXPORT_BINARY_START
global EXPORT_BINARY_END

EXPORT_BINARY_START:
    incbin "build/export.elf"
EXPORT_BINARY_END:
