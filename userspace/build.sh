#!/bin/bash
# TODO: Makefile
set -e

rm -rf build
rm -rf bin

mkdir -p build
mkdir -p bin

cflags="-O2 -I../libsyscall -ffreestanding -std=gnu99"
ldflags="-L../libsyscall -lsyscall -nostdlib"

# hello
i686-elf-gcc -c hello.c -o build/hello.o $cflags
i686-elf-gcc ../libc/bin/crt0.o build/hello.o -o bin/hello $ldflags

# fish
i686-elf-gcc -c fish.c -o build/fish.o $cflags
i686-elf-gcc ../libc/bin/crt0.o build/fish.o -o bin/fish $ldflags

# mishafetch
i686-elf-gcc -c mishafetch.c -o build/mishafetch.o $cflags
i686-elf-gcc ../libc/bin/crt0.o build/mishafetch.o -o bin/mishafetch $ldflags