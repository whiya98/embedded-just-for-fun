#ifndef PLATFORM_H
#define PLATFORM_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t baud;
    uint8_t data_bits;
    uint8_t stop_bits;
    bool parity_enabled;
    bool tx_enabled;
    bool rx_enabled;
} UartConfig;

/* Host 계약이다. TC387QP Register 또는 iLLD의 실제 API가 아니다. */
void platform_uart_apply(const UartConfig *config);
/* 요청한 prefix 중 실제로 수용한 byte 수: 0..len. 기다리지 않는다.
 * 반환한 byte는 복사 완료되어 호출 뒤 buffer를 보관하지 않는다.
 * 수용 순서는 보존한다. 물리 Pin 송신 완료를 뜻하지 않는다. */
size_t platform_uart_write(const uint8_t *bytes, size_t len);
#endif
