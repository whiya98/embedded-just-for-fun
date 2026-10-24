#ifndef FAKE_PLATFORM_H
#define FAKE_PLATFORM_H
#include "platform.h"
#define FAKE_WIRE_CAPACITY 4096u
#define FAKE_PLAN_CAPACITY 64u

typedef struct {
    UartConfig applied;
    size_t configure_calls;
    size_t write_calls;
    size_t largest_requested;
    uint8_t wire[FAKE_WIRE_CAPACITY];
    size_t wire_len;
    size_t plan[FAKE_PLAN_CAPACITY];
    size_t plan_len;
    size_t plan_at;
} FakePlatform;
extern FakePlatform fake;
void fake_reset(void);
/* 각 write의 최대 수용 수. 지정한 계획 이후에는 요청 전체를 수용한다. */
void fake_write_plan(const size_t *limits, size_t count);
#endif
