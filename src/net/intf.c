#include "intf.h"

#include "../heap.h"
#include "../string.h"

net_intf_t* net_intf_list = 0;
static net_intf_t* net_intf_last = 0;

net_intf_t* net_intf_create() {
    net_intf_t* intf = malloc(sizeof(net_intf_t));
    memset(intf, 0, sizeof(net_intf_t));
    return intf;
}

void net_intf_add(net_intf_t* intf) {
    if (!net_intf_list) {
        net_intf_list = intf;
        net_intf_last = intf;
    } else {
        net_intf_last->next = intf;
        intf->prev = net_intf_last;
        net_intf_last = intf;
    }
}