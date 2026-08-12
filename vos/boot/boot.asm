bits 32

KERNEL_PAGE_TABLES_COUNT equ 32

extern main
extern initVmm
extern removeIdentityMapping

global start
global pageDirectory
global kernelPageTables
global kernelStackPageTable

section .multiboot
align 4
    dd 0x1BADB002
    dd 0x00000000
    dd -(0x1BADB002 + 0x00000000)

section .text

start:
    call initVmm

    ; jump to higher half
    lea eax, [higherHalfEntry]
    jmp eax

higherHalfEntry:
    call removeIdentityMapping

    ; New stack pointer (Top of PDE 1022 area)
    ; PDE 1022 addresses: 0xFF800000 to 0xFFBFFFFF
    mov esp, 0xFFBFFFFC
    call main
    cli

.halt:
    hlt
    jmp .halt

section .bss
align 4096

pageDirectory:
    resb 4096

kernelPageTables:
    resb 4096 * KERNEL_PAGE_TABLES_COUNT

kernelStackPageTable:
    resb 4096
