[bits 32]

section .rodata
global UNSET_BINARY_START
global UNSET_BINARY_END

UNSET_BINARY_START:
    incbin "build/unset.elf"
UNSET_BINARY_END:
