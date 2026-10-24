#include "fake_platform.h"
#include <string.h>
FakePlatform fake;
void fake_reset(void)
{
    memset(&fake, 0, sizeof fake);
}
void fake_write_plan(const size_t *limits, size_t count)
{
    fake.plan_len = count < FAKE_PLAN_CAPACITY ? count : FAKE_PLAN_CAPACITY;
    fake.plan_at = 0;
    if (limits == NULL) {
        fake.plan_len = 0;
        return;
    }
    for (size_t i = 0; i < fake.plan_len; ++i) fake.plan[i] = limits[i];
}
void platform_uart_apply(const UartConfig *config)
{
    if (config == NULL) return;
    fake.applied = *config;
    ++fake.configure_calls;
}
size_t platform_uart_write(const uint8_t *bytes, size_t len)
{
    ++fake.write_calls;
    if (len > fake.largest_requested) fake.largest_requested = len;
    if (bytes == NULL) return 0;
    size_t accepted = len;
    if (fake.plan_at < fake.plan_len) {
        size_t limit = fake.plan[fake.plan_at++];
        if (accepted > limit) accepted = limit;
    }
    size_t room = FAKE_WIRE_CAPACITY - fake.wire_len;
    if (accepted > room) accepted = room;
    memcpy(fake.wire + fake.wire_len, bytes, accepted);
    fake.wire_len += accepted;
    return accepted;
}
