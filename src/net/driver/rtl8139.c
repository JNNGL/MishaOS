#include "rtl8139.h"

#include "../eth.h"
#include "../intf.h"
#include "../../io.h"
#include "../../isrs.h"
#include "../../heap.h"
#include "../../string.h"
#include "../../paging.h"
#include "../../terminal.h"

typedef struct rtl8139_s {
    eth_addr_t mac_address;
    uint8_t* rx_buffer;
    uint16_t io_base;
    uint8_t tx_cur;
} rtl8139_t;

static rtl8139_t device;

static uint8_t tsad_array[4] = {0x20, 0x24, 0x28, 0x2C};
static uint8_t tsd_array[4] = {0x10, 0x14, 0x18, 0x1C};

extern page_directory_t page_directory;

static uint8_t read_mac_address() {
    uint32_t value = inl(device.io_base);
    device.mac_address.address[0] = (uint8_t) value;
    device.mac_address.address[1] = (uint8_t) (value >> 8);
    device.mac_address.address[2] = (uint8_t) (value >> 16);
    device.mac_address.address[3] = (uint8_t) (value >> 24);
    value = inw(device.io_base + 0x04);
    device.mac_address.address[4] = (uint8_t) value;
    device.mac_address.address[5] = (uint8_t) (value >> 8);
    return 1;
}

void rtl8139_send(net_buf_t* buf) {
    uint32_t phys_addr = (uint32_t) pde_get_phys_addr(&page_directory, buf->start);

    outl(device.io_base + tsad_array[device.tx_cur], phys_addr);
    outl(device.io_base + tsd_array[device.tx_cur++], buf->end - buf->start);
    if (device.tx_cur > 3) {
        device.tx_cur = 0;
    }
}

void rtl8139_irq_handler(struct interrupt_frame* frame) {
    terminal_putstring("\n\nirq\n\n");
    uint16_t status = inw(device.io_base + 0x3E);

    if (status & (1 << 2)) {
        terminal_putstring("Sent packet\n");
    }

    if (status & (1 << 0)) {
        terminal_putstring("Received packet\n");
    }

    outw(device.io_base + 0x3E, 0x05);
}

void rtl8139_poll(net_intf_t* intf) {

}

void rtl8139_driver_init(pci_device_info_t* info, uint32_t bus, uint32_t dev, uint32_t func) {
    return; // TODO

    if (info->vendor_id != 0x10EC) {
        return;
    }

    if (info->device_id != 0x8139) {
        return;
    }

    terminal_putstring("Initializing RTL8139...\n");

    uint32_t id = pci_get_id(bus, dev, func);

    pci_bar_t bar;
    pci_get_bar(&bar, id, 0);

    if ((bar.flags & PCI_BAR_IO) == 0) {
        terminal_putstring("Only port based I/O supported.\n");
        return;
    }

    device.io_base = bar._.port;

    uint32_t pci_status = pci_read32(id, PCI_CONFIG_COMMAND);
    pci_status |= 4;
    pci_write32(id, PCI_CONFIG_COMMAND, pci_status);

    device.tx_cur = 0;

    outb(device.io_base + 0x52, 0x00);

    outb(device.io_base + 0x37, 0x10);
    while ((inb(device.io_base + 0x37) & 0x10) != 0);

    device.rx_buffer = malloc(8192 + 16);
    memset(device.rx_buffer, 0, 8192 + 16);
    uint32_t rx_phys_addr = (uint32_t) pde_get_phys_addr(&page_directory, device.rx_buffer);

    outl(device.io_base + 0x30, rx_phys_addr);
    outw(device.io_base + 0x3C, 0x0005);
    outl(device.io_base + 0x44, 0x0F | (1 << 7));
    outb(device.io_base + 0x37, 0x0C);

    uint8_t irq = pci_read8(id, PCI_CONFIG_INTERRUPT_LINE) - 9;
    peripheral_isrs[irq] = rtl8139_irq_handler;

    read_mac_address();

    char str[20];
    ethtoa(&device.mac_address, str);
    terminal_putstring("MAC address: ");
    terminal_putstring(str);
    terminal_putchar('\n');

    net_intf_t* intf = net_intf_create();
    intf->eth_addr = device.mac_address;
    intf->ip_addr = ipv4_null_addr;
    intf->name = "eth";
    intf->poll = rtl8139_poll;
    intf->send = eth_intf_send;
    intf->dev_send = rtl8139_send;

    net_intf_add(intf);
}