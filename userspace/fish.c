#include <syscall.h>
#include <stdint.h>
#include <stddef.h>

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) {
        len++;
    }

    return len;
}

size_t memrev(char* buf, size_t n) {
    size_t i;
    for (i = 0; i < n / 2; ++i) {
        const char x = buf[i];
        buf[i] = buf[n - i - 1];
        buf[n - i - 1] = x;
    }

    return i;
}

size_t strrev(char* s) {
    return memrev(s, strlen(s));
}

char* _itoa(int32_t target, char* buf, uint32_t radix) {
    if (radix < 2 || radix > 36) {
        return 0;
    }

    int sign = 0;
    if (target < 0) {
        sign = 1;
        target = -target;
    }

    char* low = buf;
    do {
        *buf++ = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[target % radix];
    } while (target /= radix);

    if (sign) {
        *buf++ = '-';
    }

    *buf = '\0';
    strrev(low);
    return buf;
}

static char itoa_buffer[25];
char* itoa(int32_t target, uint32_t radix) {
    _itoa(target, itoa_buffer, radix);
    return itoa_buffer;
}

void readline(char* buf) {
    while (1) {
        int r = sys_read(3, buf, 1023);

        for (uint32_t i = 0; i < r; i++) {
            if (buf[i] == '\n') {
                buf[i] = 0;
                return;
            }
        }

        buf += r;
    }
}

int main(int argc, char** argv) {
    char working_dir[1024] = {'/', 0};

    int fish_pid = sys_getpid();
    char buf[1024] = {'/', 'b', 'i', 'n', '/'};
    while (1) {
        sys_print("Current PID: ");
        sys_print(itoa(sys_getpid(), 10));
        sys_print(" (fish) [");
        sys_print(working_dir);
        sys_print("]# ");
        readline(buf + 5);

        if (buf[5] == 0) {
            continue;
        }

//        int child = sys_fork();
//        if (child == 0) {
            if (sys_exec(buf, 0) == -1) {
                sys_print("fish: command not found: ");
                sys_print(buf + 5);
                sys_print("\n");
            }

//            return -1;
//        }
    }

    return 0;
}
