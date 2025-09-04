print:
    push bp
    mov bp, sp
    pusha
    mov si, [bp+4]
    mov bh, 0x00
    mov bl, 0x00
    mov ah, 0x0E

.char:
    mov al, [si]
    inc si
    or al, al
    je .return
    int 0x10
    jmp .char

.return:
    popa
    mov sp, bp
    pop bp
    ret
