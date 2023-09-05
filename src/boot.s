.set ALIGN, 1<<0
.set MEMINFO, 1<<1
.set GFX, 1<<2
.set FLAGS, ALIGN | MEMINFO | GFX
.set MAGIC, 0x1BADB002
.set CHECKSUM, -(MAGIC + FLAGS)

.extern kernel_physical_start
.extern kernel_physical_end
.extern bss_start

.section .multiboot
.align 4
multiboot:
    .long MAGIC
    .long FLAGS
    .long CHECKSUM
    .long multiboot
    .long kernel_physical_start
    .long bss_start
    .long kernel_physical_end
    .long _start
    .long 0x00000000
    .long 0
    .long 0
    .long 32

.section .bss
.align 16
.skip 16384

stack_top:

.section .text
.global _start
.type _start, @function
_start:
    mov $stack_top, %esp
    push %esp
    push %eax
    push %ebx
    push $kernel_virtual_end
    push $kernel_virtual_start
    push $kernel_physical_end
    push $kernel_physical_start
    xor %ebp, %ebp
    cli
    call kernel_main
1:  hlt
    jmp 1b

.size _start, . - _start

.global read_eip
.type reap_eip, @function
read_eip:
    pop %eax
    jmp *%eax

.extern syscall_handle
.global syscall_handler
syscall_handler:
    cli
    push $0x00
    push $0x80
	pusha
	push %ds
	push %es
	push %fs
	push %gs
	mov $0x10, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	mov %esp, %eax
	push %eax
	call syscall_handle
	pop %eax
	pop %gs
	pop %fs
	pop %es
	pop %ds
	popa
	add $8, %esp
	iret
