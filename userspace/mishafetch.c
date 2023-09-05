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

static inline void cpuid(uint32_t reg, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx) {
    __asm__ volatile("cpuid"
                     : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
                     : "0"(reg));
}

int main(int argc, char** argv) {
    sys_print("(fish) [/]#\n(fish) [/]# mishafetch\n");
    const char* lines[] = {
            "                 ---                ",
            "  ___======____=---=)               ",
            "/T            \\_--===)              ",
            "[ \\ (0)   \\~    \\_-==)              ",
            " \\      / )J~~    \\-=)              ",
            "  \\\\___/  )JJ~~~   \\)               ",
            "   \\_____/JJJ~~~~    \\              ",
            "   / \\  , \\J~~~~~     \\             ",
            "  (-\\)\\=|\\\\\\~~~~       L__          ",
            "  (\\\\)  (\\\\\\)_           \\==__      ",
            "   \\V    \\\\\\) ===_____   \\\\\\\\\\\\     ",
            "          \\V)     \\_) \\\\\\\\JJ\\J\\)    ",
            "                      /J\\JT\\JJJJ)   ",
            "                      (JJJ| \\UUU)   ",
            "                       (UU)         ",
    };

    for (int i = 0; i < sizeof(lines) / sizeof(*lines); i++) {
        sys_print(lines[i]);

        switch (i) {
            case 0: {
                sys_write(2, "unknown", 7);
                sys_print("@");
                sys_write(2, "unknown", 7);
                break;
            }

            case 1: {
                sys_print("---------------");
                break;
            }

            case 2: {
                sys_write(2, "OS", 2);
                sys_print(": MishaOS");
                break;
            }

            case 3: {
                sys_write(2, "Kernel", 6);
                sys_print(": MishaOS");
                break;
            }

            case 4: {
                sys_write(2, "Uptime", 6);
                sys_print(": N/A");
                break;
            }

            case 5: {
                sys_write(2, "Shell", 5);
                sys_print(": fish");
                break;
            }

            case 6: {
                uint32_t width;
                uint32_t height;
                sys_screenres(&width, &height);
                sys_write(2, "Resolution", 10);
                sys_print(": ");
                sys_print(itoa(width, 10));
                sys_print("x");
                sys_print(itoa(height, 10));
                break;
            }

            case 7: {
                sys_write(2, "Font", 4);
                sys_print(": zap-vga16");
                break;
            }

            case 8: {
                sys_write(2, "CPU", 3);
                sys_print(": ");

                uint32_t unused;
                uint32_t func;
                cpuid(0x80000000, &func, &unused, &unused, &unused);

                char name[48];
                if (func >= 0x80000004) {
                    cpuid(0x80000002, (uint32_t*) name, (uint32_t*) (name +  4), (uint32_t*) (name +  8), (uint32_t*) (name + 12));
                    cpuid(0x80000003, (uint32_t*) (name + 16), (uint32_t*) (name + 20), (uint32_t*) (name + 24), (uint32_t*) (name + 28));
                    cpuid(0x80000004, (uint32_t*) (name + 32), (uint32_t*) (name + 36), (uint32_t*) (name + 40), (uint32_t*) (name + 44));

                    const char* name_ptr = name;
                    while (*name_ptr == ' ') {
                        ++name_ptr;
                    }

                    sys_print(name_ptr);
                } else {
                    cpuid(0, &unused, (uint32_t*) name, (uint32_t*) (name + 8), (uint32_t*) (name + 4));
                    name[12] = '\0';

                    sys_print(name);
                }

                break;
            }

            case 9: {
                uint32_t used;
                uint32_t total;
                sys_meminfo(&used, &total);
                sys_write(2, "Memory", 6);
                sys_print(": ");
                sys_print(itoa(used / 1024 / 1024, 10));
                sys_print("MiB / ");
                sys_print(itoa(total / 1024 / 1024, 10));
                sys_print("MiB");
            }
        }

        sys_print("\n");
    }

    return 0;
}