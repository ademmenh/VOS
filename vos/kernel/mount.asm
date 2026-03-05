[bits 32]

section .rodata
global MOUNT_BINARY_START
global MOUNT_BINARY_END

MOUNT_BINARY_START:
    incbin "build/mount.elf"
MOUNT_BINARY_END:
