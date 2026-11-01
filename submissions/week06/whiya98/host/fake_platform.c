#include "fake_platform.h"
#include <string.h>

FakePlatform fake;

static void record(char code)
{
    if (fake.trace_length + 1u < sizeof fake.trace) {
        fake.trace[fake.trace_length++] = code;
        fake.trace[fake.trace_length] = '\0';
    }
}

static void dispatch_pending(void)
{
    /* 1-bit Pending 모델이다. 실제 Hardware의 모든 미처리 주기를 세는 Queue가 아니다. */
    while (fake.irq_enabled && !fake.in_isr && fake.irq_pending) {
        uint32_t tick = fake.pending_tick;
        bool previous = fake.irq_enabled;
        fake.irq_pending = false;
        fake.irq_enabled = false;
        fake.in_isr = true;
        fake.now = tick;
        fake.isr_calls++;
        record('I');
        timer_isr();
        record('X');
        fake.in_isr = false;
        fake.irq_enabled = previous;
    }
}

void fake_raise_irq(uint32_t tick)
{
    fake.irq_pending = true;
    fake.pending_tick = tick;
    dispatch_pending();
}

static void run_hook(HookPoint point)
{
    /* Hook은 Main의 지정 지점에서 한 번만 소비한다. ISR 재진입은 없다. */
    if (!fake.in_isr && fake.hook == point) {
        uint32_t tick = fake.hook_tick;
        fake.hook = HOOK_NONE;
        fake_raise_irq(tick);
    }
}

void fake_arm_hook(HookPoint point, uint32_t tick)
{
    fake.hook = point;
    fake.hook_tick = tick;
}

void fake_reset(void)
{
    memset(&fake, 0, sizeof fake);
    event_reset(); /* IRQ disabled, Timer stopped 상태이다. */
    fake.irq_enabled = true;
    fake.sensor_value = UINT16_C(1234);
}

void fake_clear_observations(void)
{
    fake.isr_calls = 0u;
    fake.service_calls = 0u;
    fake.now_calls = 0u;
    fake.sensor_calls = 0u;
    fake.publish_calls = 0u;
    fake.delay_calls = 0u;
    fake.save_calls = 0u;
    fake.restore_calls = 0u;
    fake.interleave_calls = 0u;
    fake.io_in_isr = 0u;
    fake.io_irq_disabled = 0u;
    fake.service_outside_isr = 0u;
    memset(&fake.last_published, 0, sizeof fake.last_published);
    fake.trace_length = 0u;
    fake.trace[0] = '\0';
}

bool platform_irq_save_disable(void)
{
    bool previous = fake.irq_enabled;
    fake.save_calls++;
    record('L');
    fake.irq_enabled = false;
    return previous;
}

void platform_irq_restore(bool previous_enabled)
{
    fake.restore_calls++;
    record('U');
    fake.irq_enabled = previous_enabled;
    dispatch_pending();
}

void platform_timer_service(void)
{
    fake.service_calls++;
    record('S');
    if (!fake.in_isr) { fake.service_outside_isr++; }
}

uint32_t platform_timer_now(void)
{
    fake.now_calls++;
    record('N');
    return fake.now;
}

static void observe_heavy_io(void)
{
    if (fake.in_isr) { fake.io_in_isr++; }
    if (!fake.irq_enabled) { fake.io_irq_disabled++; }
}

uint16_t platform_sensor_read(void)
{
    uint16_t value = fake.sensor_value;
    fake.sensor_calls++;
    record('R');
    observe_heavy_io();
    run_hook(HOOK_SENSOR);
    return value;
}

void platform_publish(const SampleReport *report)
{
    fake.publish_calls++;
    record('P');
    observe_heavy_io();
    fake.last_published = *report;
    run_hook(HOOK_PUBLISH);
}

void platform_delay_ms(uint32_t milliseconds)
{
    (void)milliseconds;
    fake.delay_calls++;
    record('D');
    observe_heavy_io();
}

void platform_interleave(void)
{
    fake.interleave_calls++;
    record('H');
    run_hook(HOOK_INTERLEAVE);
}
