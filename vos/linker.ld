ENTRY(start)

KERNEL_OFFSET = 0xC0000000;

SECTIONS {
    . = 0x00100000;

    .multiboot : {
        *(.multiboot)
    }

    . += KERNEL_OFFSET;

    KERNEL_START = .;

    .text ALIGN(4K) : AT(ADDR(.text) - KERNEL_OFFSET) {
        *(.text)
    }

    .rodata ALIGN(4K) : AT(ADDR(.rodata) - KERNEL_OFFSET) {
        *(.rodata)
    }

    .data ALIGN(4K) : AT(ADDR(.data) - KERNEL_OFFSET) {
        *(.data)
    }

    .bss ALIGN(4K) : AT(ADDR(.bss) - KERNEL_OFFSET) {
        *(.bss)
        *(COMMON)
    }

    KERNEL_END = .;
}
