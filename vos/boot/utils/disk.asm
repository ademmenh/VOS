; usage:
; call init_disk
init_disk:
    mov ah, 0
    mov dl, 0x80
    int 0x13
    jc boot

; usage
; push 0x100        es segment
; push 17           number of sectors
; push 0            cylinder index
; push 0            sector index
; push 0            head index
; call read_disk
; add sp, 10
read_disk:
    pusha
    ; setup the extra segment
    mov ax, [bp+10]
    mov es, ax
    xor bx, bx

    mov ah, 0x02
    ; read disk
    mov dl, 0x80
    ; number of sectors
    mov al, [bp+8]
    ; cylinder index
    mov ch, [bp+6]
    ; sector index
    mov cl, [bp+4]
    ; head index
    mov dh, [bp+2]
    int 0x13
    popa
    ret