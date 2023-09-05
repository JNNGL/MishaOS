#include "panic.h"

#include <lib/kprintf.h>
#include <kernel.h>

static uint8_t _panic = 0;

extern uintptr_t read_eip();

void panic(const char* msg) {
    if (++_panic == 1) {
        terminal_init();

        puts("\n!! KERNEL PANIC !!");
        puts(msg);

        puts("\nStack Trace: ");
        struct stackframe {
            struct stackframe* ebp;
            uint32_t eip;
        };

        struct stackframe* stack;
        asm("movl %%ebp, %0" : "=r"(stack));
        for (uint32_t frame = 0; stack && frame < 20; ++frame) {
            kprintf(" 0x%08lx: %s\n", stack->eip, kernel_get_func_name(stack->eip));
            stack = stack->ebp;
        }
    } else if (_panic == 2) {
        puts("Panic has caused another panic. Something bad happened.");
    } else {
        goto halt;
    }

    puts("\n!! FISH IS NOT OPTIMIZED !!");

    halt:
    for (;;) {
        asm("hlt");
    }
}