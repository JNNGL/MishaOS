#pragma once

#include <sys/process.h>

#define SYS_EXIT 0
#define SYS_PRINT 1
#define SYS_YIELD 2
#define SYS_WRITE 3
#define SYS_READ 4
#define SYS_GETPID 5
#define SYS_FORK 6
#define SYS_EXEC 7
#define SYS_ISPROCALIVE 8
#define SYS_MEMINFO 9
#define SYS_SCREENRES 10

void syscall_handle(struct syscall_regs* registers);