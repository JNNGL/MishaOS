#include "driver.h"

#include <dev/net/intel.h>
#include <dev/net/rtl8139.h>
//#include <dev/usb/ehci.h>
//#include <dev/usb/uhci.h>

const driver_t INTEL_NET_DRIVER = {.init = intel_net_driver_init};
const driver_t RTL8139_NET_DRIVER = {.init = rtl8139_driver_init};

//const driver_t EHCI_DRIVER = {.init = ehci_init};
//const driver_t UHCI_DRIVER = {.init = uhci_init};

const driver_t* PCI_DRIVERS[] = {
        &INTEL_NET_DRIVER,
        &RTL8139_NET_DRIVER,
//        &EHCI_DRIVER,
//        &UHCI_DRIVER,
        0
};