newline:
    push ax
    push bx

    mov ah, 0x0E
    mov bh, 0x00
    mov bl, 0x00

    mov al, 0x0D
    int 0x10

    mov al, 0x0A
    int 0x10

    pop bx
    pop ax
    jmp .char
    ret
