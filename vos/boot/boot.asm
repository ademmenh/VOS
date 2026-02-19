bits 32

PAGE_PRESENT    equ (1 << 0)
PAGE_WRITABLE   equ (1 << 1)
PAGE_USER       equ (1 << 2)
PAGE_WRITETHRU  equ (1 << 3)
PAGE_CACHE_DIS  equ (1 << 4)
PAGE_ACCESSED   equ (1 << 5)
PAGE_DIRTY      equ (1 << 6)
PAGE_4MB        equ (1 << 7)
PAGE_GLOBAL     equ (1 << 8)

KERNEL_OFFSET equ 0xC0000000

extern main
extern KERNEL_END

global start
global pageDirectory
global pageTable
global stackPageTable

section .multiboot
align 4
    dd 0x1BADB002
    dd 0x00000000
    dd -(0x1BADB002 + 0x00000000)

section .text

start:
    cli
    ; 1:1 mapping
    mov edi, pageTable - KERNEL_OFFSET
    mov eax, PAGE_PRESENT | PAGE_WRITABLE
    mov ecx, 1024
    .fillPT:
    mov [edi], eax
    add eax, 4096
    add edi, 4
    dec ecx
    jnz .fillPT
    
    mov edi, pageDirectory - KERNEL_OFFSET
    xor eax, eax
    mov ecx, 1024
    .clearPD:
    mov [edi], eax
    add edi, 4
    dec ecx
    jnz .clearPD

    ; identity mapping PDE 0
    mov eax, pageTable - KERNEL_OFFSET
    or  eax, PAGE_PRESENT | PAGE_WRITABLE
    mov [pageDirectory - KERNEL_OFFSET + 0*4], eax
    
    ; map higher-half kernel to PDE 768 - 0xC0000000
    mov [pageDirectory - KERNEL_OFFSET + 768*4], eax

    ; setup stack page table
    mov edi, stackPageTable - KERNEL_OFFSET
    xor eax, eax
    mov ecx, 1024
    .clearStackPT:
    mov [edi], eax
    add edi, 4
    dec ecx
    jnz .clearStackPT

    ; map last 4 entries (1020–1023)
    mov edi, (stackPageTable - KERNEL_OFFSET) + 1020*4
    
    ; mapping kernel stack from the KERNEL_END
    mov eax, KERNEL_END
    sub eax, KERNEL_OFFSET
    ; Align to 4KB (though linker usually does it)
    add eax, 4095
    and eax, 0xFFFFF000
    or eax, PAGE_PRESENT | PAGE_WRITABLE
    mov ecx, 4
    .mapStack:
    mov [edi], eax
    add eax, 4096
    add edi, 4
    dec ecx
    jnz .mapStack

    ; attach stack page table to PDE[1023]
    mov eax, stackPageTable - KERNEL_OFFSET
    or  eax, PAGE_PRESENT | PAGE_WRITABLE
    mov [pageDirectory - KERNEL_OFFSET + 1023*4], eax

    ; set cr3 to pageDirectory
    mov eax, pageDirectory - KERNEL_OFFSET
    mov cr3, eax

    ; enable cr0.PE
    mov eax, cr0
    or  eax, 0x80000000
    mov cr0, eax

    ; jump to higher half
    lea eax, [higherHalfEntry]
    jmp eax
    

higherHalfEntry:
    ; remove identity mapping
    mov dword [pageDirectory + 0*4], 0
    mov eax, pageDirectory - KERNEL_OFFSET
    mov cr3, eax

    ; set stack at top of 4GB
    mov esp, 0xFFFFFFFF
    call main
    cli

.halt:
    hlt
    jmp .halt

section .bss
align 4096

pageDirectory:
    resb 4096

pageTable:
    resb 4096

stackPageTable:
    resb 4096
