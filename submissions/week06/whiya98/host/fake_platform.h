#ifndef FAKE_PLATFORM_H
#define FAKE_PLATFORM_H
#include <stddef.h>
#include "platform.h"

typedef enum { HOOK_NONE, HOOK_INTERLEAVE, HOOK_SENSOR, HOOK_PUBLISH } HookPoint;
typedef struct {
    bool irq_enabled;
    bool in_isr;
    bool irq_pending;
    uint32_t pending_tick;
    uint32_t now;
    uint16_t sensor_value;
    HookPoint hook;
    uint32_t hook_tick;
    unsigned isr_calls;
    unsigned service_calls;
    unsigned now_calls;
    unsigned sensor_calls;
    unsigned publish_calls;
    unsigned delay_calls;
    unsigned save_calls;
    unsigned restore_calls;
    unsigned interleave_calls;
    unsigned io_in_isr;
    unsigned io_irq_disabled;
    unsigned service_outside_isr;
    SampleReport last_published;
    char trace[512];
    size_t trace_length;
} FakePlatform;

extern FakePlatform fake;
void fake_reset(void);
void fake_clear_observations(void);
void fake_raise_irq(uint32_t tick);
void fake_arm_hook(HookPoint point, uint32_t tick);
#endif
