#include <netif.h>

void setup() {
    // Initialize both interfaces
    netif_init(NetIf::wifi);
    netif_init(NetIf::eth);
}

void loop() {
    // Poll both interfaces
    netif_poll(NetIf::wifi);
    netif_poll(NetIf::eth);
}