global gdt_flush

gdt_flush:
    mov eax, [esp+4]
    lgdt [eax]

    mov eax, 0x10
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    jmp 0x08:.flush

    .flush:
        ret