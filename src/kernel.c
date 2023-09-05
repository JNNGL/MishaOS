#include "kernel.h"

#include <lib/kprintf.h>
#include <lib/tga.h>
#include <lib/string.h>
#include <cpu/gdt.h>
#include <cpu/tss.h>
#include <cpu/idt.h>
#include <cpu/pic.h>
#include <cpu/acpi.h>
#include <cpu/paging.h>
#include <dev/pci.h>
#include <dev/storage/ide.h>
#include <dev/input/mouse.h>
#include <sys/kernel_mem.h>
#include <sys/panic.h>
#include <sys/pit.h>
#include <sys/heap.h>
#include <sys/isrs.h>
#include <sys/syscall.h>
#include <sys/exec.h>
#include <sys/mount.h>
#include <sys/process.h>
#include <net/net.h>
#include <net/intf.h>
#include <video/mouse_renderer.h>
#include <video/graphics.h>
#include <video/vga_terminal.h>
#include <video/lfb_terminal.h>
#include <video/lfb.h>
#include <multiboot.h>
#include <vfs.h>

kernel_func_info_t* kernel_funcs = 0;

const char* kernel_get_func_name(uintptr_t addr) {
    if (!kernel_funcs) {
        return "??";
    }

    kernel_func_info_t* func = kernel_funcs;
    while (func->start_addr > addr || func->end_addr <= addr) {
        func = (kernel_func_info_t*) ((uint8_t*) func + func->len);
        if (func->len == 0) {
            return "??";
        }
    }

    return func->func_name;
}

void kernel_main(kernel_meminfo_t meminfo, struct multiboot* multiboot, uint32_t multiboot_msg, uint32_t esp) {
    terminal = vga_terminal;
    terminal_init();
    puts("Optimizing fish...");

    uint32_t cursor_buffer[CURSOR_WIDTH * CURSOR_HEIGHT + 2];

    uint32_t module_start = 0;
    uint32_t module_end = 0;
    vfs_filesystem_t initrd;

    if (multiboot_msg == MULTIBOOT_EAX_MAGIC) {
        puts("Multiboot structure is valid.");
        if (!(multiboot->flags >> 6 & 0x1)) {
            panic("Invalid memory map");
        }

        if (multiboot->mods_count == 0) {
            panic("Initial ramdisk not found.");
        }

        module_start = *(uint32_t*) multiboot->mods_addr;
        module_end = *(uint32_t*) (multiboot->mods_addr + 4);
        kprintf("Found initrd: %lu bytes\n", module_end - module_start);

        puts("Loading initrd...");

        vfs_read_filesystem(&initrd, (uint8_t*) module_start);
        vfs_entry_t* test_entry = vfs_find_entry(&initrd, ".initrd_test");
        if (!test_entry || memcmp("optimizedfish", vfs_file_content(&initrd, test_entry, 0), test_entry->size)) {
            panic("Failed to verify initrd");
        }

        mount_root(&initrd, vfs_find_entry(&initrd, "."));

        vbe_info_t* vbe_info = ((vbe_info_t*) (multiboot->vbe_mode_info));
        linear_framebuffer = (uint8_t*) vbe_info->physbase;
        lfb_width = vbe_info->x_res;
        lfb_height = vbe_info->y_res;

        puts("Initialized linear framebuffer.");
    } else {
        panic("Invalid multiboot header.");
    }

    // 0x1000000 (16M) -> page bitmap
    // 0x1020000 (16M + 128K) -> back framebuffer
    uint32_t* back_framebuffer = (uint32_t*) 0x1020000;

    vfs_entry_t* funcs = vfs_find_entry(&initrd, ".funcs");
    if (funcs) {
        kernel_funcs = (kernel_func_info_t*) vfs_file_content(&initrd, funcs, 0);
        puts("Loaded kernel functions."); // TODO: DWARF
    }

    vfs_entry_t* logo = vfs_find_entry(&initrd, "logo.tga");
    tga_parse(back_framebuffer, vfs_file_content(&initrd, logo, 0), logo->size);

    uint32_t logo_width = back_framebuffer[0];
    uint32_t logo_height = back_framebuffer[1];
    uint32_t logo_x = lfb_width / 2 - logo_width / 2;
    uint32_t logo_y = lfb_height / 2 - logo_height / 2;
    for (int i = logo_height - 1; i >= 0; i--) {
        void* reloc_address = back_framebuffer + lfb_width * i + logo_y * lfb_width + logo_x;
        void* load_address = back_framebuffer + logo_width * i + 2;
        memcpy(reloc_address, load_address, logo_width * 4);
    }

    for (uint32_t i = 0; i < lfb_height; i++) {
        if (i <= logo_y || i >= logo_y + logo_height) {
            memset(back_framebuffer + i * lfb_width, 0, lfb_width * 4);
        } else {
            memset(back_framebuffer + i * lfb_width, 0, logo_x * 4);
            memset(back_framebuffer + i * lfb_width + logo_x + logo_width, 0, (lfb_width - logo_width - logo_x) * 4);
        }
    }

    memcpy(linear_framebuffer, back_framebuffer, lfb_width * lfb_height * 4);
    lfb_set_back_buffer(back_framebuffer);

    vfs_entry_t* font = vfs_find_entry(&initrd, "zap-vga16.psf");
    psf_font_t* psf_font = psf_load_font(vfs_file_content(&initrd, font, 0));

    terminal = lfb_terminal;
    terminal_init();
    lfb_terminal_set_font(psf_font);
    lfb_copy_from_vga();

    rsdp_t* rsdp = rsdp_locate(multiboot);
    if (rsdp) {
        char rsdp_string[7];
        memcpy(rsdp_string, &rsdp->oem_id, 6);
        rsdp_string[6] = 0;
        kprintf("[ACPI] Found RSDP [%s]", rsdp_string);
        if (rsdp->revision == 0) {
            puts(" version 1.0");
            acpi_parse_rsdt((sdt_header_t*) rsdp->rsdt_address);
        } else if (rsdp->revision == 2) {
            puts(" version 2.0");
            rsdp2_t* rsdp2 = (rsdp2_t*) rsdp;
            if (rsdp2->xsdt_address) {
                acpi_parse_xsdt((sdt_header_t*)(uintptr_t) rsdp2->xsdt_address);
            } else {
                acpi_parse_rsdt((sdt_header_t*)(uintptr_t) rsdp->rsdt_address);
            }
        } else {
            panic("[ACPI] Unsupported ACPI version");
        }
    } else {
        puts("[ACPI] RSDP not found");
    }

    gdt_entry_t gdt[6];

    puts("Loading GDT...");
    gdt_encode_entry(&gdt[0], 0, 0, 0, 0);
    gdt_encode_entry(&gdt[1], 0, 0xFFFFFFFF, 0x9A, 0xCF);
    gdt_encode_entry(&gdt[2], 0, 0xFFFFFFFF, 0x92, 0xCF);
    gdt_encode_entry(&gdt[3], 0, 0xFFFFFFFF, 0xFA, 0xCF);
    gdt_encode_entry(&gdt[4], 0, 0xFFFFFFFF, 0xF2, 0xCF);
    tss_encode_entry(&gdt[5], 0x10, esp);
    gdt_load(sizeof(gdt) - 1, (uint32_t) &gdt);
    tss_flush();

    puts("Loading IDT...");
    idt_entry_t idt[256];
    idt_encode_entry(&idt[0x08], (uint32_t) double_fault_isr, 0x08, 0, 0xE);
    idt_encode_entry(&idt[0x0D], (uint32_t) general_protection_fault_isr, 0x08, 0, 0xE);
    idt_encode_entry(&idt[0x0E], (uint32_t) page_fault_isr, 0x08, 0, 0xE);
    idt_encode_entry(&idt[0x20], (uint32_t) pit_isr, 0x08, 0, 0xE);
    idt_encode_entry(&idt[0x21], (uint32_t) keyboard_isr, 0x08, 0, 0xE);
    idt_encode_entry(&idt[0x29], (uint32_t) peripheral_handler0, 0x08, 0, 0xE);
    idt_encode_entry(&idt[0x2A], (uint32_t) peripheral_handler1, 0x08, 0, 0xE);
    idt_encode_entry(&idt[0x2B], (uint32_t) peripheral_handler2, 0x08, 0, 0xE);
    idt_encode_entry(&idt[0x2C], (uint32_t) ps2_mouse_isr, 0x08, 0, 0xE);
    idt_encode_entry(&idt[0x80], (uint32_t) syscall_handler, 0x08, 3, 0xE);
    idt_load(sizeof(idt) - 1, (uint32_t) &idt);

    puts("Initializing paging...");
    pfa_read_memory_map(&pfa, multiboot, &meminfo, module_start, module_end);

    // Lock pages
    for (uint32_t i = (uint32_t) back_framebuffer; i <= (uint32_t) back_framebuffer + lfb_height * lfb_width * 4; i += 0x1000) {
        pfa_lock_page(&pfa, (void*) i);
    }

    for (uint32_t i = (uint32_t) linear_framebuffer; i < (uint32_t) linear_framebuffer + lfb_height * lfb_width * 4 + 0x1000; i += 0x1000) {
        pfa_lock_page(&pfa, (void*) i);
    }

    pde_init(&page_directory, &pfa);

    // Map pages
    for (uint32_t i = (uint32_t) back_framebuffer; i <= (uint32_t) back_framebuffer + lfb_height * lfb_width * 4; i += 0x1000) {
        pde_map_memory(&page_directory, &pfa, (void*) i, (void*) i);
    }

    for (uint32_t i = (uint32_t) linear_framebuffer; i < (uint32_t) linear_framebuffer + lfb_height * lfb_width * 4 + 0x1000; i += 0x1000) {
        pde_map_memory(&page_directory, &pfa, (void*) i, (void*) i);
    }

    multiboot_memory_map_t* mmap_entry = (multiboot_memory_map_t*) multiboot->mmap_addr;
    while ((uint32_t) mmap_entry < multiboot->mmap_addr + multiboot->mmap_length) {
        for (uint64_t i = 0; i < mmap_entry->len; i += 0x1000) {
            uint64_t address = mmap_entry->addr + i;
            if (address > UINT32_MAX - 0x1000) {
                goto next;
            }


            void* ptr = (void*)(uint32_t) address;
            pde_map_memory(&page_directory, &pfa, ptr, ptr);
        }

        next:
        mmap_entry = (multiboot_memory_map_t*) (((uint32_t) mmap_entry) + mmap_entry->size + sizeof(mmap_entry->size));
    }

    enable_paging(&page_directory);

    puts("Initializing heap..."); // TODO: Rewrite heap
    heap_init(0x100); // TODO: 64-bit kernel for larger address space

    puts("Remapping PIC...");
    pic_remap(0x20, 0x28);

    puts("Initializing mouse...");
    vfs_entry_t* cursors = vfs_find_entry(&initrd, "cursors");
    vfs_entry_t* cursor = vfs_find_entry_in(&initrd, cursors, "center_ptr.tga");
    tga_parse(cursor_buffer, vfs_file_content(&initrd, cursor, 0), cursor->size);
    if (cursor_buffer[0] != CURSOR_WIDTH || cursor_buffer[1] != CURSOR_HEIGHT) {
        panic("Invalid cursor size.");
    }

    mouse_set_cursor(cursor_buffer + 2);
    mouse_init();

    puts("Initializing PIT...");
    pit_set_phase(1000);

    puts("Enabling IRQs...");
    pic_irq_set_master_mask(0b11111000);
    pic_irq_set_slave_mask(0b11100001);

    asm("sti");

    puts("Initializing PCI...");
    for (uint32_t bus = 0; bus < 256; ++bus) {
        for (uint32_t dev = 0; dev < 32; ++dev) {
            uint32_t base_id = pci_get_id(bus, dev, 0);
            uint8_t header_type = pci_read8(base_id, PCI_CONFIG_HEADER_TYPE);
            uint32_t func_count = header_type & PCI_TYPE_MULTIFUNC ? 8 : 1;

            for (uint32_t func = 0; func < func_count; ++func) {
                pci_visit(bus, dev, func);
            }
        }
    }

    ide_init(0x1F0, 0x3F6, 0x170, 0x376, 0x000);

    void net_post(net_intf_t* intf) {
        kprintf("Configured network interface %s.\n", intf->name);
    }

    puts("Initializing networking...");
    net_post_init = net_post;
    net_init();

    puts("Initializing multitasking...");
    init_process(esp);

    system("/bin/fish", 0, 0);

    while (1);
}

void kernel_poll() {
    mouse_handle_packet();
    graphics_redraw();
    net_poll();
}