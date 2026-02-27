section .rodata
global vsh_binary_start
global vsh_binary_end

vsh_binary_start:
    incbin "build/vsh.elf"
vsh_binary_end:
