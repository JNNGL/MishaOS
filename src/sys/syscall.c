#include "syscall.h"

#include <lib/kprintf.h>
#include <sys/exec.h>
#include <video/lfb.h>

__attribute__((noreturn))
static int sys_exit(int rval) {
    task_exit(rval);
    while (1);
}

static int sys_print(const char* str) {
    if (!str) {
        return -1; // TODO: Segmentation fault
    }

    kprintf("%s", str);
    return 0;
}

static int sys_yield() {
    switch_task(1);
    return 0;
}

static int sys_write(int fd, char* data, int len) {
    if (!data || len <= 0) {
        return -1;
    }

    process_t* process = (process_t*) current_process;
    file_descriptor_t* file = process_get_fd(process, fd);
    if (!file) {
        return -1;
    }

    return file->write(file, data, len);
}

static int sys_read(int fd, char* data, int len) {
    if (!data || len <= 0) {
        return -1;
    }

    process_t* process = (process_t*) current_process;
    file_descriptor_t* file = process_get_fd(process, fd);
    if (!file) {
        return -1;
    }

    return file->read(file, data, len);
}

static int sys_exec(const char* filename, const char* argv[]) {
    exec(filename, 0, 0);
    return -1;
}

static int sys_isprocalive(int pid) { // Temporary workaround
    return get_process(pid) != 0;
}

static int sys_meminfo(uint32_t* used, uint32_t* total) {
    *used = pfa_used_memory();
    *total = *used + pfa_free_memory();
    return 0;
}

static int sys_screenres(uint32_t* width, uint32_t* height) {
    *width = lfb_width;
    *height = lfb_height;
    return 0;
}

static uint32_t syscalls[] = {
        (uint32_t) &sys_exit,
        (uint32_t) &sys_print,
        (uint32_t) &sys_yield,
        (uint32_t) &sys_write,
        (uint32_t) &sys_read,
        (uint32_t) &getpid,
        (uint32_t) &fork,
        (uint32_t) &sys_exec,
        (uint32_t) &sys_isprocalive, // Temporary workaround
        (uint32_t) &sys_meminfo,
        (uint32_t) &sys_screenres,
};

void syscall_handle(struct syscall_regs* registers) {
    if ((uint32_t) registers->eax >= sizeof(syscalls) / sizeof(syscalls[0])) {
        return;
    }

    uintptr_t handler = syscalls[(uint32_t) registers->eax];
    if (handler == 0) {
        return;
    }

    current_process->syscall_regs = registers;

    uint32_t ret;
    asm volatile("push %1\n"
                 "push %2\n"
                 "push %3\n"
                 "push %4\n"
                 "push %5\n"
                 "call *%6\n"
                 "pop %%ebx\n"
                 "pop %%ebx\n"
                 "pop %%ebx\n"
                 "pop %%ebx\n"
                 "pop %%ebx\n"
                 : "=a"(ret)
                 : "r"(registers->edi), "r"(registers->esi)
                 , "r"(registers->edx), "r"(registers->ecx)
                 , "r"(registers->ebx), "r"(handler));

    registers = current_process->syscall_regs;
    registers->eax = ret;
}