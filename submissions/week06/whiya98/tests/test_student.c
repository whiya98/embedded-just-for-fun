
#include "test_support.h"
#include "mission.h"
#include "fake_platform.h"
#include "platform.h"

/* Test 1: IRQ Disabled 중 여러 요청 발생 */
static void test_multiple_irqs_while_disabled(void)
{
    fake_reset();

    bool previous = platform_irq_save_disable();

    fake_raise_irq(10u);
    fake_raise_irq(20u);
    fake_raise_irq(30u);

    // IRQ 차단 중에는 ISR이 실행되지 않음
    CHECK(fake.isr_calls == 0u);
    CHECK(fake.irq_pending);
    CHECK(fake.pending_tick == 30u);
    CHECK(g_events.pending == 0u);

    platform_irq_restore(previous);

    // IRQ 복원 후 마지막 요청 하나만 처리
    CHECK(fake.irq_enabled);
    CHECK(fake.isr_calls == 1u);
    CHECK(!fake.irq_pending);
    CHECK(g_events.pending == 1u);
    CHECK(g_events.dropped == 0u);
    CHECK(g_events.last_tick == 30u);
}

/* Test 2: Publish 중 새 요청이 발생하는 경우 */
static void test_irq_during_publish(void)
{
    fake_reset();

    // 최초 요청 2개
    fake_raise_irq(10u);
    fake_raise_irq(20u);

    fake.sensor_value = 2048u;

    // Publish 도중 tick 30 요청 발생
    fake_arm_hook(HOOK_PUBLISH, 30u);

    SampleReport out = {0};

    CHECK(collector_step(&out));

    // 첫 번째 Report는 기존 요청 2개만 반영
    CHECK(out.requests == 2u);
    CHECK(out.coalesced == 1u);
    CHECK(out.request_tick == 20u);
    CHECK(out.dropped == 0u);
    CHECK(out.value == 2048u);

    // Publish된 Report도 동일해야 함
    CHECK(fake.last_published.requests == 2u);
    CHECK(fake.last_published.request_tick == 20u);
    CHECK(fake.last_published.value == 2048u);

    // 새로운 요청은 Mailbox에 남아야 함
    CHECK(g_events.pending == 1u);
    CHECK(g_events.last_tick == 30u);
    CHECK(fake.io_in_isr == 0u);
    CHECK(fake.io_irq_disabled == 0u);

    // 두 번째 Main 처리
    CHECK(collector_step(&out));

    CHECK(out.requests == 1u);
    CHECK(out.coalesced == 0u);
    CHECK(out.request_tick == 30u);

    CHECK(fake.sensor_calls == 2u);
    CHECK(fake.publish_calls == 2u);
    CHECK(g_events.pending == 0u);
}

/* 학생 추가 테스트 실행 */
void test_student_cases(void)
{
    test_multiple_irqs_while_disabled();
    test_irq_during_publish();
}
