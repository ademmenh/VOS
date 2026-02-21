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

PAGING_ENABLE   equ (1 << 31)

KERNEL_OFFSET equ 0xC0000000

extern main
extern KERNEL_END

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
    cli
    ; kernelPageTables mapping - 128MB (32 PDEs)
    mov edi, kernelPageTables - KERNEL_OFFSET
    mov eax, PAGE_PRESENT | PAGE_WRITABLE
    mov ecx, 1024 * 32
    .fillPT:
    mov [edi], eax
    add eax, 4096
    add edi, 4
    dec ecx
    jnz .fillPT
    
    ; clear PageDirectory
    mov edi, pageDirectory - KERNEL_OFFSET
    xor eax, eax
    mov ecx, 1024
    .clearPD:
    mov [edi], eax
    add edi, 4
    dec ecx
    jnz .clearPD

    ; Map 32 PDEs for identity mapping and higher-half
    mov edi, pageDirectory - KERNEL_OFFSET
    mov eax, kernelPageTables - KERNEL_OFFSET
    or  eax, PAGE_PRESENT | PAGE_WRITABLE
    mov ecx, 32
    .fillPDLinks:
    mov [edi], eax                ; Identity map
    mov [edi + 768*4], eax        ; Higher half kernel
    add eax, 4096
    add edi, 4
    dec ecx
    jnz .fillPDLinks

    ; clear kernelStackPageTable
    mov edi, kernelStackPageTable - KERNEL_OFFSET
    xor eax, eax
    mov ecx, 1024
    .clearKernelStackPT:
    mov [edi], eax
    add edi, 4
    dec ecx
    jnz .clearKernelStackPT

    ; map kernelStackPageTable from the KERNEL_END
    mov edi, (kernelStackPageTable - KERNEL_OFFSET) + 1020*4
    mov eax, KERNEL_END - KERNEL_OFFSET
    add eax, 4095
    and eax, 0xFFFFF000
    or eax, PAGE_PRESENT | PAGE_WRITABLE
    mov ecx, 4
    .mapKernelStack:
    mov [edi], eax
    add eax, 4096
    add edi, 4
    dec ecx
    jnz .mapKernelStack

    ; PDE[1022] mapping - kernelStackPageTable (RELOCATED)
    mov eax, kernelStackPageTable - KERNEL_OFFSET
    or  eax, PAGE_PRESENT | PAGE_WRITABLE
    mov [pageDirectory - KERNEL_OFFSET + 1022*4], eax

    ; PDE[1023] mapping - RECURSIVE PAGING!
    mov eax, pageDirectory - KERNEL_OFFSET
    or  eax, PAGE_PRESENT | PAGE_WRITABLE
    mov [pageDirectory - KERNEL_OFFSET + 1023*4], eax

    ; set cr3 to pageDirectory
    mov eax, pageDirectory - KERNEL_OFFSET
    mov cr3, eax

    ; enable cr0.PE
    mov eax, cr0
    or  eax, PAGING_ENABLE
    mov cr0, eax

    ; jump to higher half
    lea eax, [higherHalfEntry]
    jmp eax
    

higherHalfEntry:
    ; remove identity mapping
    mov edi, pageDirectory
    xor eax, eax
    mov ecx, 32
    rep stosd

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
    resb 4096 * 32

kernelStackPageTable:
    resb 4096
