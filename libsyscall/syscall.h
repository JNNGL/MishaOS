#pragma once

#include <stdint.h>

void sys_exit(int rval);
int sys_print(const char* msg);
void sys_yield();
int sys_write(int fd, char* buf, int len);
int sys_read(int fd, char* buf, int len);
int sys_getpid();
int sys_fork();
int sys_exec(const char* filename, const char* argv[]);
int sys_isprocalive(int pid); // Temporary workaround
int sys_meminfo(uint32_t* used, uint32_t* total);
int sys_screenres(uint32_t* width, uint32_t* height);