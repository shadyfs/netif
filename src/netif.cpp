// src/netif.cpp
#include "netif.h"

// backend hooks
void netif_wifi_init();
void netif_wifi_poll();
bool netif_wifi_is_up();

void netif_eth_init();
void netif_eth_poll();
bool netif_eth_is_up();

void netif_init(NetIf iface) {
    switch (iface) {
        case NetIf::wifi:
            netif_wifi_init();
            break;
        case NetIf::eth:
            netif_eth_init();
            break;
    }
}

void netif_poll(NetIf iface) {
    switch (iface) {
        case NetIf::wifi:
            netif_wifi_poll();
            break;
        case NetIf::eth:
            netif_eth_poll();
            break;
    }
}

bool netif_is_up(NetIf iface) {
    switch (iface) {
        case NetIf::wifi:
            return netif_wifi_is_up();
        case NetIf::eth:
            return netif_eth_is_up();
    }
    return false;
}
