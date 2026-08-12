[bits 32]

KERNEL_PAGE_TABLES_COUNT equ 32

PAGE_PRESENT    equ (1 << 0)
PAGE_RW         equ (1 << 1)
PAGE_USER       equ (1 << 2)
PAGE_WRITETHRU  equ (1 << 3)
PAGE_CACHE_DIS  equ (1 << 4)
PAGE_ACCESSED   equ (1 << 5)
PAGE_DIRTY      equ (1 << 6)
PAGE_4MB        equ (1 << 7)
PAGE_GLOBAL     equ (1 << 8)

PAGING_ENABLE   equ (1 << 31)

KERNEL_OFFSET   equ 0xC0000000

extern kernelPageTables
extern pageDirectory
extern kernelStackPageTable
extern KERNEL_END

global initVmm
global removeIdentityMapping
global enablePaging
global invalidatePage

section .text

initVmm:
    cli
    ; kernelPageTables mapping - KERNEL_PAGE_TABLES_COUNT PDEs
    mov edi, kernelPageTables - KERNEL_OFFSET
    mov eax, PAGE_PRESENT | PAGE_RW
    mov ecx, 1024 * KERNEL_PAGE_TABLES_COUNT
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

    ; Map KERNEL_PAGE_TABLES_COUNT PDEs for identity mapping and higher-half
    mov edi, pageDirectory - KERNEL_OFFSET
    mov eax, kernelPageTables - KERNEL_OFFSET
    or  eax, PAGE_PRESENT | PAGE_RW
    mov ecx, KERNEL_PAGE_TABLES_COUNT
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
    or eax, PAGE_PRESENT | PAGE_RW
    mov ecx, 4
    .mapKernelStack:
    mov [edi], eax
    add eax, 4096
    add edi, 4
    dec ecx
    jnz .mapKernelStack

    ; PDE[1022] mapping - kernelStackPageTable (RELOCATED)
    mov eax, kernelStackPageTable - KERNEL_OFFSET
    or  eax, PAGE_PRESENT | PAGE_RW
    mov [pageDirectory - KERNEL_OFFSET + 1022*4], eax

    ; PDE[1023] mapping - RECURSIVE PAGING!
    mov eax, pageDirectory - KERNEL_OFFSET
    or  eax, PAGE_PRESENT | PAGE_RW
    mov [pageDirectory - KERNEL_OFFSET + 1023*4], eax

    ; set cr3 to pageDirectory
    mov eax, pageDirectory - KERNEL_OFFSET
    mov cr3, eax

    ; enable cr0.PE
    mov eax, cr0
    or  eax, PAGING_ENABLE
    mov cr0, eax
    ret

removeIdentityMapping:
    ; remove identity mapping
    mov edi, pageDirectory
    xor eax, eax
    mov ecx, KERNEL_PAGE_TABLES_COUNT
    rep stosd
    ret

enablePaging:
    push ebp
    mov ebp, esp
    mov eax, [ebp + 8]
    mov cr3, eax

    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    mov esp, ebp
    pop ebp
    ret

invalidatePage:
    mov eax, [esp + 4]
    invlpg [eax]
    ret

global getCR2
getCR2:
    mov eax, cr2
    ret

global getCR3
getCR3:
    mov eax, cr3
    ret
