#pragma once

#include <stdint.h>
#include <stddef.h>
#include <cpu/paging.h>
#include <lib/tree.h>

struct vfs_entry_s;

typedef int32_t pid_t;
typedef uint8_t status_t;

struct syscall_regs {
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t int_no;
    uint32_t err_code;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t useresp;
    uint32_t ss;
};

enum wait_option {
    WCONTINUED,
    WNOHANG,
    WUNTRACED,
};

struct process_s;

struct process_queue {
    struct process_queue* next;
    struct process_queue* prev;
    struct process_s* process;
    uint8_t queued;
};

typedef struct thread_s {
    uintptr_t esp;
    uintptr_t ebp;
    uintptr_t eip;
    uint8_t reserved[108];
    uint8_t padding[32];
    page_directory_t* page_directory;
} thread_t;

typedef struct ximage_s {
    size_t size;
    uintptr_t entry;
    uintptr_t heap;
    uintptr_t heap_aligned;
    uintptr_t stack;
    uintptr_t user_stack;
    uintptr_t start;
} ximage_t;

typedef struct file_descriptor_s {
    int(*read)(struct file_descriptor_s* fd, void* buf, size_t len);
    int(*write)(struct file_descriptor_s* fd, void* buf, size_t len);
    int(*close)(struct file_descriptor_s* fd);
    void* context;
    size_t position;
    size_t length;
} file_descriptor_t;

typedef struct fd_list_s {
    struct fd_list_s* link;
    file_descriptor_t* value;
    uint32_t fd;
} fd_list_t;

typedef struct process_s {
    pid_t id;
    char* name;
    thread_t thread;
    ximage_t image;
    tree_t* process_tree;
    char* working_dir_path;
    struct vfs_entry_s* working_dir_entry;
    fd_list_t* fds;
    file_descriptor_t* stdout;
    file_descriptor_t* stderr;
    file_descriptor_t* stdin;
    uint32_t current_fd;
    status_t status;
    uint8_t finished;
    uint8_t started;
    struct syscall_regs* syscall_regs;
    struct process_queue queue_node;
    struct process_queue reap_node;
} process_t;

void init_process(uint32_t esp);
process_t* spawn_process(volatile process_t* parent);
process_t* spawn_init(uint32_t esp);
void set_process_page_directory(process_t* process, page_directory_t* page_directory);
void make_process_ready(process_t* process);
void make_process_reapable(process_t* process);
uint8_t process_available();
process_t* next_ready_process();
uint8_t should_reap();
process_t* next_reapable_process();
void reap_process(process_t* process);
uint32_t process_add_fd(process_t* process, file_descriptor_t* file);
process_t* get_process(pid_t pid);
void delete_process(process_t* process);
file_descriptor_t* process_get_fd(process_t* process, uint32_t fd);
uint32_t process_clone_fd(process_t* process, int from, int to);
int process_is_ready(process_t* process);

__attribute__((returns_twice)) pid_t fork();
__attribute__((returns_twice)) pid_t clone(uintptr_t new_stack, uintptr_t thread_func, uintptr_t arg);
pid_t getpid();

void switch_task(uint8_t reschedule);
void switch_next();
void enter_userspace(uintptr_t entry, uintptr_t stack, int argc, const char** argv);
void task_exit(int r);

extern volatile process_t* current_process;