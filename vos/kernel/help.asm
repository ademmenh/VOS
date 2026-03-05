[bits 32]

section .rodata
global HELP_BINARY_START
global HELP_BINARY_END

HELP_BINARY_START:
    incbin "build/help.elf"
HELP_BINARY_END:
