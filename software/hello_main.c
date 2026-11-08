#include <stdint.h>
#include "hello_identity.h"

#if !HELLO_IDENTITY_CONFIGURED
#error "Edit hello_identity.h with your team name and VM IP, then set HELLO_IDENTITY_CONFIGURED to 1"
#endif

static void puts_uart(const char *s) {
    while (*s) {
        *(volatile uint32_t *)(uintptr_t)0x40600004 = (uint8_t)*s++;
        __asm__ volatile("fence iorw,iorw" ::: "memory");
    }
}

int main(void) {
    /* IP is recorded from the host VM, not discovered by this bare-metal program. */
    puts_uart("hello xiangshan, I am " HELLO_TEAM ", IP address " HELLO_VM_IP "\n");
    return 0;
}
