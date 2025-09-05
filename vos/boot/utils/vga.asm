; push mode
; push msg
; call writeToVga
; add sp, 4
writeToVga:
    push bp
    mov bp, sp
    pusha
    
    ; vga memory segment
    mov ax, 0xb800
    mov es, ax
    
    mov bx, [bp+4]
    mov ah, [bp+6]
    xor si, si

    .char:
        mov al, [bx]
        ; chekcs the end of the string 
        or al, al
        je .ret
        
        mov [es:si], ax
        inc bx
        add si, 2
        jmp .char

    .ret:
        popa
        pop bp
        ret