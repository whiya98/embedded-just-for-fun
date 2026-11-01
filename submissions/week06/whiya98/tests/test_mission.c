#include "mission.h"
#include "fake_platform.h"
#include "test_support.h"
#include <stdio.h>
#include <string.h>

static unsigned checks;
static unsigned failures;

void test_check(bool result, const char *expression, const char *file, int line)
{
    checks++;
    if (!result) {
        failures++;
        printf("FAIL %s:%d: %s\n", file, line, expression);
    }
}

static void seed(uint32_t pending, uint32_t dropped, uint32_t tick)
{
    /* Unit Test의 유효 상태 준비. 동시 실행 중의 생산 코드가 아니다. */
    g_events.pending = pending;
    g_events.dropped = dropped;
    g_events.last_tick = tick;
}

static bool same_batch(EventBatch a, EventBatch b)
{
    return a.pending == b.pending && a.dropped == b.dropped && a.last_tick == b.last_tick;
}

static bool same_report(SampleReport a, SampleReport b)
{
    return a.request_tick == b.request_tick && a.requests == b.requests &&
           a.coalesced == b.coalesced && a.dropped == b.dropped && a.value == b.value;
}

static void check_mailbox(uint32_t pending, uint32_t dropped, uint32_t tick)
{
    CHECK(g_events.pending == pending);
    CHECK(g_events.dropped == dropped);
    CHECK(g_events.last_tick == tick);
}

static void test_post_policy(void)
{
    fake_reset();
    check_mailbox(0u, 0u, 0u);
    for (uint32_t i = 1u; i <= 6u; ++i) {
        /* event_post_isr만 검증하기 위해 Timer ISR 문맥을 Fixture로 설정한다. */
        fake.in_isr = true;
        fake.irq_enabled = false;
        event_post_isr(i * 10u);
        fake.in_isr = false;
        fake.irq_enabled = true;
        check_mailbox(i <= 4u ? i : 4u, i <= 4u ? 0u : i - 4u, i <= 4u ? i * 10u : 40u);
    }
    CHECK(fake.trace_length == 0u);
    seed(4u, UINT32_MAX - 1u, 99u);
    event_post_isr(100u);
    check_mailbox(4u, UINT32_MAX, 99u);
    event_post_isr(101u);
    check_mailbox(4u, UINT32_MAX, 99u);
}

static void test_take_null_empty_and_previous_irq(void)
{
    EventBatch sentinel = {91u, 92u, 93u};
    EventBatch out = sentinel;
    fake_reset();
    seed(4u, 1u, 40u);
    fake_arm_hook(HOOK_INTERLEAVE, 30u);
    CHECK(!event_take(NULL));
    check_mailbox(4u, 1u, 40u);
    CHECK(fake.trace_length == 0u);
    CHECK(fake.hook == HOOK_INTERLEAVE);
    CHECK(fake.irq_enabled);

    for (unsigned enabled = 0u; enabled <= 1u; ++enabled) {
        fake_reset();
        fake.irq_enabled = enabled != 0u;
        out = sentinel;
        CHECK(!event_take(&out));
        CHECK(same_batch(out, sentinel));
        check_mailbox(0u, 0u, 0u);
        CHECK(fake.irq_enabled == (enabled != 0u));
        CHECK(fake.save_calls == 1u);
        CHECK(fake.restore_calls == 1u);
        CHECK(fake.interleave_calls == 1u);
        CHECK(fake.sensor_calls == 0u && fake.publish_calls == 0u);
        seed(4u, 2u, 40u);
        fake_clear_observations();
        CHECK(event_take(&out));
        CHECK(out.pending == 4u && out.dropped == 2u && out.last_tick == 40u);
        check_mailbox(0u, 0u, 0u);
        CHECK(fake.irq_enabled == (enabled != 0u));
        CHECK(fake.save_calls == 1u && fake.restore_calls == 1u);
        CHECK(fake.interleave_calls == 1u);
    }
}

static void test_snapshot_interleave(void)
{
    EventBatch out = {91u, 92u, 93u};
    fake_reset();
    seed(4u, 1u, 40u);
    fake_arm_hook(HOOK_INTERLEAVE, 50u);
    CHECK(event_take(&out));
    CHECK(out.pending == 4u && out.dropped == 1u && out.last_tick == 40u);
    check_mailbox(1u, 0u, 50u);
    CHECK(fake.isr_calls == 1u);
    CHECK(fake.interleave_calls == 1u);
    CHECK(fake.irq_enabled);
    CHECK(!fake.irq_pending);
    CHECK(event_take(&out));
    CHECK(out.pending == 1u && out.dropped == 0u && out.last_tick == 50u);
    check_mailbox(0u, 0u, 0u);

    fake_reset();
    fake.irq_enabled = false;
    seed(1u, 0u, 10u);
    fake_arm_hook(HOOK_INTERLEAVE, 20u);
    CHECK(event_take(&out));
    CHECK(out.pending == 1u && out.last_tick == 10u);
    CHECK(!fake.irq_enabled);
    CHECK(fake.irq_pending);
    CHECK(fake.isr_calls == 0u);
    check_mailbox(0u, 0u, 0u);
    platform_irq_restore(true);
    CHECK(fake.isr_calls == 1u);
    check_mailbox(1u, 0u, 20u);

    fake_reset();
    EventBatch sentinel = {91u, 92u, 93u};
    out = sentinel;
    fake_arm_hook(HOOK_INTERLEAVE, 10u);
    CHECK(!event_take(&out));
    CHECK(same_batch(out, sentinel));
    check_mailbox(1u, 0u, 10u);
    CHECK(fake.isr_calls == 1u);
}

static void test_short_isr(void)
{
    fake_reset();
    fake_raise_irq(10u);
    CHECK(strcmp(fake.trace, "ISNX") == 0);
    CHECK(fake.service_calls == 1u);
    CHECK(fake.now_calls == 1u);
    CHECK(fake.service_outside_isr == 0u);
    CHECK(fake.sensor_calls == 0u);
    CHECK(fake.publish_calls == 0u);
    CHECK(fake.delay_calls == 0u);
    CHECK(fake.io_in_isr == 0u);
    CHECK(fake.irq_enabled && !fake.in_isr);
    check_mailbox(1u, 0u, 10u);
    fake_clear_observations();
    seed(4u, UINT32_MAX, 40u);
    fake_raise_irq(50u);
    CHECK(strcmp(fake.trace, "ISNX") == 0);
    CHECK(fake.service_calls == 1u);
    check_mailbox(4u, UINT32_MAX, 40u);
}

static void test_collector_contract(void)
{
    SampleReport sentinel = {91u, 92u, 93u, 94u, 95u};
    SampleReport out = sentinel;
    fake_reset();
    seed(1u, 0u, 10u);
    CHECK(!collector_step(NULL));
    CHECK(fake.trace_length == 0u);
    check_mailbox(1u, 0u, 10u);
    fake_reset();
    CHECK(!collector_step(&out));
    CHECK(same_report(out, sentinel));
    CHECK(fake.sensor_calls == 0u && fake.publish_calls == 0u);
    CHECK(fake.interleave_calls == 1u);

    const uint16_t values[] = {0u, 1234u, 4095u};
    for (size_t v = 0u; v < sizeof values / sizeof values[0]; ++v) {
        for (uint32_t n = 1u; n <= 4u; ++n) {
            fake_reset();
            seed(n, n == 4u ? 2u : 0u, n * 10u);
            fake.now = 500u;
            fake.sensor_value = values[v];
            out = sentinel;
            CHECK(collector_step(&out));
            CHECK(out.request_tick == n * 10u);
            CHECK(out.requests == n);
            CHECK(out.coalesced == n - 1u);
            CHECK(out.dropped == (n == 4u ? 2u : 0u));
            CHECK(out.value == values[v]);
            CHECK(same_report(out, fake.last_published));
            CHECK(fake.sensor_calls == 1u);
            CHECK(fake.publish_calls == 1u);
            CHECK(fake.interleave_calls == 1u);
            CHECK(fake.io_in_isr == 0u);
            CHECK(fake.io_irq_disabled == 0u);
            CHECK(fake.delay_calls == 0u);
            CHECK(fake.now_calls == 0u);
            CHECK(fake.irq_enabled);
            check_mailbox(0u, 0u, 0u);
            SampleReport previous = out;
            fake_clear_observations();
            CHECK(!collector_step(&out));
            CHECK(same_report(out, previous));
            CHECK(fake.sensor_calls == 0u && fake.publish_calls == 0u);
        }
    }
}

static void test_new_irq_during_main_io(void)
{
    const HookPoint hooks[] = {HOOK_SENSOR, HOOK_PUBLISH};
    for (size_t i = 0u; i < sizeof hooks / sizeof hooks[0]; ++i) {
        fake_reset();
        seed(3u, 0u, 30u);
        fake.now = 35u;
        fake_arm_hook(hooks[i], 40u);
        SampleReport out = {0};
        CHECK(collector_step(&out));
        CHECK(out.requests == 3u && out.coalesced == 2u && out.request_tick == 30u);
        CHECK(fake.sensor_calls == 1u && fake.publish_calls == 1u);
        CHECK(fake.io_irq_disabled == 0u && fake.io_in_isr == 0u);
        CHECK(fake.isr_calls == 1u);
        check_mailbox(1u, 0u, 40u);
        CHECK(collector_step(&out));
        CHECK(out.requests == 1u && out.coalesced == 0u && out.request_tick == 40u);
        CHECK(fake.sensor_calls == 2u && fake.publish_calls == 2u);
        check_mailbox(0u, 0u, 0u);
    }
}

static void test_end_to_end_burst(void)
{
    fake_reset();
    for (uint32_t tick = 10u; tick <= 60u; tick += 10u) { fake_raise_irq(tick); }
    check_mailbox(4u, 2u, 40u);
    CHECK(fake.isr_calls == 6u && fake.service_calls == 6u);
    CHECK(fake.sensor_calls == 0u && fake.publish_calls == 0u);
    fake.now = 65u;
    SampleReport out = {0};
    CHECK(collector_step(&out));
    CHECK(out.requests == 4u && out.coalesced == 3u && out.dropped == 2u);
    CHECK(out.request_tick == 40u);
    CHECK(fake.sensor_calls == 1u && fake.publish_calls == 1u);
    CHECK(fake.io_in_isr == 0u && fake.io_irq_disabled == 0u);
    check_mailbox(0u, 0u, 0u);
}

int main(void)
{
    test_post_policy();
    test_take_null_empty_and_previous_irq();
    test_snapshot_interleave();
    test_short_isr();
    test_collector_contract();
    test_new_irq_during_main_io();
    test_end_to_end_burst();
    test_student_cases();
    printf("%u checks, %u failures\n", checks, failures);
    return failures == 0u ? 0 : 1;
}
