// src/netif.h
#pragma once
#include <stdint.h>

enum class NetIf : uint8_t {
    wifi,
    eth
};

// lifecycle
void netif_init(NetIf iface);
void netif_poll(NetIf iface);

// status (optional but useful)
bool netif_is_up(NetIf iface);