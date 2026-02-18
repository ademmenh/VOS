global loadGDT

loadGDT:
    mov eax, [esp+4]
    lgdt [eax]
    ; far jump to flush pipeline and load CS
    jmp 0x08:protected_mode

protected_mode:
    mov ax, 0x10
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ret
