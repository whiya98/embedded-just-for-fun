#include "mission.h"
#include "platform.h"
void uart_configure(void)
{
    const UartConfig config = {
        .baud = 9600u,
        .data_bits = 8u,
        .stop_bits = 1u,
        .parity_enabled = false,
        .tx_enabled = true,
        .rx_enabled = false
    };
    platform_uart_apply(&config);
}
