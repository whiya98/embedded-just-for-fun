#ifndef PLATFORM_H
#define PLATFORM_H
#include <stdbool.h>
#include <stdint.h>
#include "mission.h"

/* save는 이전 상태를 반환하며 IRQ를 금지한다. restore는 그 상태를 복원한다.
 * 실제 Target 구현에는 Compiler ordering과 필요한 Architecture 보장을 포함한다. */
bool platform_irq_save_disable(void);
void platform_irq_restore(bool previous_enabled);
/* Host에서는 호출 순서를 기록한다. Target의 다음 Compare 준비 등을 추상화하며,
 * 특정 CMP Flag Clear의 필요성이나 지연된 여러 주기의 복원을 보장하지 않는다. */
void platform_timer_service(void);
uint32_t platform_timer_now(void);
uint16_t platform_sensor_read(void);
void platform_publish(const SampleReport *report);
void platform_delay_ms(uint32_t milliseconds);
/* Mailbox Snapshot 뒤, Clear 전에 정확히 한 번 유지해야 하는 Test Hook. */
void platform_interleave(void);
#endif
