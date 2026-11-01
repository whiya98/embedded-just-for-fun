#ifndef MISSION_H
#define MISSION_H
#include <stdbool.h>
#include <stdint.h>

#define EVENT_CAPACITY UINT32_C(4)

typedef struct {
    volatile uint32_t pending;
    volatile uint32_t dropped;
    volatile uint32_t last_tick;
} EventMailbox;

typedef struct {
    uint32_t pending;
    uint32_t dropped;
    uint32_t last_tick;
} EventBatch;

typedef struct {
    uint32_t request_tick;
    uint32_t requests;
    uint32_t coalesced;
    uint32_t dropped;
    uint16_t value;
} SampleReport;

extern EventMailbox g_events;
/* Timer가 정지했거나 IRQ가 금지된 초기화 단계에서만 호출한다. */
void event_reset(void);
/* 단일 Timer ISR에서만 호출한다. */
void event_post_isr(uint32_t tick);
/* out은 Mailbox와 겹치지 않는 독립 객체이다. NULL은 부작용 없이 거부한다. */
bool event_take(EventBatch *out);
void timer_isr(void);
/* Main / IRQ enabled 상태에서 호출한다. NULL은 부작용 없이 거부한다. */
bool collector_step(SampleReport *out);
#endif
