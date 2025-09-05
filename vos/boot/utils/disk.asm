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
    push bp
    mov bp, sp
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
    pop bp
    ret


; usage:
; push 0x100        ; destination segment 
; push 0x0000       ; destination offset
; push 17           ; number of sectors to read
; push 0x0000       ; LBA low
; push 0x0000       ; LBA high
; call read_disk_lba
; add sp, 10
read_disk_lba:
    push bp
    mov bp, sp
    pusha

    ; Setup segment registers
    mov ax, cs
    mov ds, ax

    ; number of sectors
    mov ax, [bp+10]
    mov word [DAP+2], ax


    ; destination address
    ; Offset
    mov ax, [bp+8]
    mov word [DAP+ 4], ax
    ; Segment
    mov ax, [bp+6]
    mov word [DAP + 6], ax

    ; LBA low 32 bits
    ; LBA low
    mov ax, [bp+4]
    mov word [DAP + 8], ax
    ; LBA high
    mov ax, [bp+2]
    mov word [DAP + 10], ax
    xor ax, ax
    ; LBA upper 32 bits
    mov word [DAP + 12], ax
    mov word [DAP + 14], ax

    mov si, DAP
    mov dl, 0x80
    mov ah, 0x42
    int 0x13

    popa
    pop bp
    ret