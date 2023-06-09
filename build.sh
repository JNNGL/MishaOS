#!/bin/bash
# TODO: Makefile
set -e

cc_flags="-std=gnu99 -ffreestanding -Wall -O2 -Wextra -Imishavfs -Isrc"

# Build sources
mkdir -p build
mkdir -p build/net
mkdir -p build/net/driver
mkdir -p build/usb
mkdir -p build/gui
mkdir -p build/mc
i686-elf-as src/boot.s -o build/boot.o
i686-elf-gcc -c src/kernel.c -o build/kernel.o $cc_flags
i686-elf-gcc -c src/terminal.c -o build/terminal.o $cc_flags
i686-elf-as src/gdt.s -o build/gdt_s.o
i686-elf-gcc -c src/gdt.c -o build/gdt.o $cc_flags
i686-elf-as src/idt.s -o build/idt_s.o
i686-elf-gcc -c src/idt.c -o build/idt.o $cc_flags
i686-elf-gcc -c src/gpd.c -o build/gpd.o $cc_flags -O0
i686-elf-gcc -c src/isrs.c -o build/isrs.o $cc_flags -mgeneral-regs-only -Wno-unused-parameter
i686-elf-gcc -c src/panic.c -o build/panic.o $cc_flags
i686-elf-gcc -c src/string.c -o build/string.o $cc_flags
i686-elf-gcc -c src/pic.c -o build/pic.o $cc_flags
i686-elf-gcc -c src/io.c -o build/io.o $cc_flags
i686-elf-gcc -c src/acpi.c -o build/acpi.o $cc_flags
i686-elf-gcc -c src/stdlib.c -o build/stdlib.o $cc_flags
i686-elf-gcc -c src/pci.c -o build/pci.o $cc_flags
i686-elf-gcc -c src/driver.c -o build/driver.o $cc_flags
i686-elf-gcc -c src/ide.c -o build/ide.o $cc_flags
i686-elf-gcc -c mishavfs/vfs.c -o build/vfs.o $cc_flags
i686-elf-gcc -c src/vga_terminal.c -o build/vga_terminal.o $cc_flags
i686-elf-gcc -c src/lfb.c -o build/lfb.o $cc_flags
i686-elf-gcc -c src/psf_font.c -o build/psf_font.o $cc_flags
i686-elf-gcc -c src/lfb_terminal.c -o build/lfb_terminal.o $cc_flags
i686-elf-gcc -c src/tga.c -o build/tga.o $cc_flags
i686-elf-gcc -c src/mouse.c -o build/mouse.o $cc_flags
i686-elf-gcc -c src/pit.c -o build/pit.o $cc_flags
i686-elf-gcc -c src/sleep.c -o build/sleep.o $cc_flags
i686-elf-gcc -c src/paging.c -o build/paging.o $cc_flags -O0
i686-elf-gcc -c src/heap.c -o build/heap.o $cc_flags -O0
i686-elf-gcc -c src/ctype.c -o build/ctype.o $cc_flags
i686-elf-gcc -c src/time.c -o build/time.o $cc_flags
i686-elf-gcc -c src/rtc.c -o build/rtc.o $cc_flags
i686-elf-gcc -c src/kprintf.c -o build/kprintf.o $cc_flags
i686-elf-gcc -c src/net/addr.c -o build/net/addr.o $cc_flags
i686-elf-gcc -c src/net/buf.c -o build/net/buf.o $cc_flags
i686-elf-gcc -c src/net/eth.c -o build/net/eth.o $cc_flags
i686-elf-gcc -c src/net/intf.c -o build/net/intf.o $cc_flags
i686-elf-gcc -c src/net/ipv4.c -o build/net/ipv4.o $cc_flags
i686-elf-gcc -c src/net/route.c -o build/net/route.o $cc_flags
i686-elf-gcc -c src/net/checksum.c -o build/net/checksum.o $cc_flags
i686-elf-gcc -c src/net/arp.c -o build/net/arp.o $cc_flags
i686-elf-gcc -c src/net/net.c -o build/net/net.o $cc_flags
i686-elf-gcc -c src/net/loopback.c -o build/net/loopback.o $cc_flags
i686-elf-gcc -c src/net/dns.c -o build/net/dns.o $cc_flags
i686-elf-gcc -c src/net/udp.c -o build/net/udp.o $cc_flags
i686-elf-gcc -c src/net/port.c -o build/net/port.o $cc_flags
i686-elf-gcc -c src/net/dhcp.c -o build/net/dhcp.o $cc_flags
i686-elf-gcc -c src/net/ntp.c -o build/net/ntp.o $cc_flags
i686-elf-gcc -c src/net/tcp.c -o build/net/tcp.o $cc_flags
i686-elf-gcc -c src/net/icmp.c -o build/net/icmp.o $cc_flags
i686-elf-gcc -c src/net/driver/intel.c -o build/net/driver/intel.o $cc_flags
i686-elf-gcc -c src/net/driver/rtl8139.c -o build/net/driver/rtl8139.o $cc_flags
#i686-elf-gcc -c src/usb/controller.c -o build/usb/controller.o $cc_flags
#i686-elf-gcc -c src/usb/desc.c -o build/usb/desc.o $cc_flags
#i686-elf-gcc -c src/usb/dev.c -o build/usb/dev.o $cc_flags
#i686-elf-gcc -c src/usb/driver.c -o build/usb/driver.o $cc_flags
#i686-elf-gcc -c src/usb/ehci.c -o build/usb/ehci.o $cc_flags
#i686-elf-gcc -c src/usb/uhci.c -o build/usb/uhci.o $cc_flags
i686-elf-gcc -c src/gui/mouse_renderer.c -o build/gui/mouse_renderer.o $cc_flags
i686-elf-gcc -c src/gui/gui.c -o build/gui/gui.o $cc_flags
i686-elf-gcc -c src/mc/f3f5.c -o build/mc/f3f5.o $cc_flags
i686-elf-gcc -c src/mc/mcprotocol.c -o build/mc/mcprotocol.o $cc_flags

objects="build/idt_s.o \
                build/idt.o \
                build/gpd.o \
                build/boot.o \
                build/kernel.o \
                build/terminal.o \
                build/gdt_s.o \
                build/gdt.o \
                build/isrs.o \
                build/panic.o \
                build/string.o \
                build/io.o \
                build/pic.o \
                build/acpi.o \
                build/stdlib.o \
                build/pci.o \
                build/driver.o \
                build/ide.o \
                build/vfs.o \
                build/vga_terminal.o \
                build/lfb.o \
                build/psf_font.o \
                build/lfb_terminal.o \
                build/tga.o \
                build/mouse.o \
                build/sleep.o \
                build/pit.o \
                build/paging.o \
                build/heap.o \
                build/ctype.o \
                build/time.o \
                build/rtc.o \
                build/kprintf.o \
                build/net/addr.o \
                build/net/buf.o \
                build/net/eth.o \
                build/net/intf.o \
                build/net/ipv4.o \
                build/net/checksum.o \
                build/net/route.o \
                build/net/arp.o \
                build/net/net.o \
                build/net/loopback.o \
                build/net/dns.o \
                build/net/udp.o \
                build/net/port.o \
                build/net/dhcp.o \
                build/net/ntp.o \
                build/net/tcp.o \
                build/net/icmp.o \
                build/net/driver/intel.o \
                build/net/driver/rtl8139.o \
                build/gui/mouse_renderer.o \
                build/gui/gui.o \
                build/mc/f3f5.o \
                build/mc/mcprotocol.o"

i686-elf-gcc -T linker.ld -o build/mishaos.bin -ffreestanding -O2 -nostdlib $objects -lgcc

#        build/usb/controller.o \
#        build/usb/desc.o \
#        build/usb/dev.o \
#        build/usb/driver.o \
#        build/usb/ehci.o \
#        build/usb/uhci.o \

# Build mishavfs
pushd mishavfs
./build.sh
popd

# Create initrd
rm -f build/initrd.img
mishavfs/build/misha.mkvfs --label=INITRD initrd build/initrd.img

pushd mishaboot/lgbt
./build.sh
popd

pushd mishaboot/eboot
./build.sh
popd

# Create bootable image
mkdir -p build/iso/boot/grub
cp -f grub.cfg build/iso/boot/grub
cp -f build/mishaos.bin build/iso/boot
cp -f build/initrd.img build/iso/boot
grub-mkrescue -o mishaos_grub.iso build/iso