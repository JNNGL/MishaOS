.global _start
.extern main
_start:
    pop %eax
    call main
    mov %eax, %ebx
    # TODO: sys_exit here
.loop:
    hlt
    jmp .loop
