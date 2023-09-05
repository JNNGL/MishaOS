#include "syscall.h"

#include <sys/syscall.h>

#define arg_ptr(p) ((uint32_t)(uintptr_t) p)

void sys_exit(int rval) {
    __asm__ __volatile__("int $0x80" : : "a"(SYS_EXIT), "b"(rval));
}

int sys_print(const char* msg) {
    int eax;
    __asm__ __volatile__("int $0x80" : "=a"(eax) : "0"(SYS_PRINT), "b"(arg_ptr(msg)));
    return eax;
}

void sys_yield() {
    __asm__ __volatile__("int $0x80" : : "a"(SYS_YIELD));
}

int sys_write(int fd, char* buf, int len) {
    int eax;
    __asm__ __volatile__("int $0x80" : "=a"(eax) : "0"(SYS_WRITE), "b"(fd), "c"(arg_ptr(buf)), "d"(len));
    return eax;
}

int sys_read(int fd, char* buf, int len) {
    int eax;
    __asm__ __volatile__("int $0x80" : "=a"(eax) : "0"(SYS_READ), "b"(fd), "c"(arg_ptr(buf)), "d"(len));
    return eax;
}

int sys_getpid() {
    int eax;
    __asm__ __volatile__("int $0x80" : "=a"(eax) : "0"(SYS_GETPID));
    return eax;
}

int sys_fork() {
    int eax;
    __asm__ __volatile__("int $0x80" : "=a"(eax) : "0"(SYS_FORK));
    return eax;
}

int sys_exec(const char* filename, const char* argv[]) {
    int eax;
    __asm__ __volatile__("int $0x80" : "=a"(eax) : "0"(SYS_EXEC), "b"(arg_ptr(filename)), "c"(arg_ptr(argv)));
    return eax;
}

int sys_isprocalive(int pid) {
    int eax;
    __asm__ __volatile__("int $0x80" : "=a"(eax) : "0"(SYS_ISPROCALIVE), "b"(pid));
    return eax;
}

int sys_meminfo(uint32_t* used, uint32_t* total) {
    int eax;
    __asm__ __volatile__("int $0x80" : "=a"(eax) : "0"(SYS_MEMINFO), "b"(used), "c"(total));
    return eax;
}

int sys_screenres(uint32_t* width, uint32_t* height) {
    int eax;
    __asm__ __volatile__("int $0x80" : "=a"(eax) : "0"(SYS_SCREENRES), "b"(width), "c"(height));
    return eax;
}