ENTRY(start)

KERNEL_VMA = 0xC0000000;

SECTIONS {
    . = 0x00100000;

    .multiboot : {
        *(.multiboot)
    }

    . += KERNEL_VMA;

    KERNEL_START = .;

    .text ALIGN(4K) : AT(ADDR(.text) - KERNEL_VMA) {
        *(.text)
    }

    .rodata ALIGN(4K) : AT(ADDR(.rodata) - KERNEL_VMA) {
        *(.rodata)
    }

    .data ALIGN(4K) : AT(ADDR(.data) - KERNEL_VMA) {
        *(.data)
    }

    .bss ALIGN(4K) : AT(ADDR(.bss) - KERNEL_VMA) {
        *(.bss)
        *(COMMON)
    }

    KERNEL_END = .;
}
