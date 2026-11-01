#include "mission.h"
#include "fake_platform.h"
#include <inttypes.h>
#include <stdio.h>

static void show(const char *scenario, uint32_t main_tick)
{
    SampleReport out = {0};
    fake.now = main_tick;
    bool ready = collector_step(&out);
    printf("%s,%" PRIu32 ",%d,%" PRIu32 ",%" PRIu32 ",%" PRIu32 ",%" PRIu32 ",%u,%" PRIu32 ",%u,%u,%u,%u\n",
           scenario, main_tick, ready ? 1 : 0, out.request_tick, out.requests,
           out.coalesced, out.dropped, (unsigned)out.value, g_events.pending,
           fake.sensor_calls, fake.publish_calls, fake.io_in_isr, fake.io_irq_disabled);
}

int main(void)
{
    puts("scenario,main_tick,report,request_tick,requests,coalesced,dropped,value,next_pending,sensor_calls,publish_calls,io_in_isr,io_irq_disabled");
    fake_reset();
    show("empty", 0u);
    fake_raise_irq(10u);
    show("normal", 12u);
    show("already_taken", 15u);
    fake_reset();
    for (uint32_t tick = 10u; tick <= 30u; tick += 10u) { fake_raise_irq(tick); }
    show("late_main", 35u);
    fake_reset();
    for (uint32_t tick = 10u; tick <= 60u; tick += 10u) { fake_raise_irq(tick); }
    show("overflow", 65u);
    fake_reset();
    fake_raise_irq(10u);
    fake_arm_hook(HOOK_INTERLEAVE, 20u);
    show("snapshot_race", 15u);
    show("next_request", 22u);
    return 0;
}
