; push msg
; call print
; add sp, 2
print:
    push bp
    mov bp, sp
    pusha
    mov bx, [bp+4]
    mov ah, 0x0E

    .char:
        mov al, [bx]
        inc bx 
        or al, al
        je .return
        int 0x10
        jmp .char

    .return:
        popa
        mov sp, bp
        pop bp
        ret


; call new line
newline:
    push ax
    push bx

    mov ah, 0x0E
    mov al, 0x0D
    int 0x10

    mov al, 0x0A
    int 0x10

    pop bx
    pop ax
    ret
