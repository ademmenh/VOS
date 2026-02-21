section .text

global contextSwitch
global taskTrampoline

taskTrampoline:
    call edi
    jmp $

global userTrampoline
userTrampoline:
    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    iret

contextSwitch:
    mov eax, [esp + 4]
    mov edx, [esp + 8]
    pusha

    mov [eax], esp
    mov esp, edx

    popa
    ret