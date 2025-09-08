global outb
global inb
global inw
global io_wait

outb:
    mov edx, [esp+4]
    mov al, [esp+8]
    out dx, al
    ret

inb:
    mov edx, [esp+4]
    in al, dx
    movzx eax, al
    ret

inw:
    mov edx, [esp+4]
    in ax, dx
    movzx eax, ax
    ret

; void io_wait(void)
io_wait:
    mov al, 0
    out 0x80, al
    ret
