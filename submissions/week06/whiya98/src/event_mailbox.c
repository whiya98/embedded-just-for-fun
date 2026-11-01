#include "mission.h"
#include "platform.h"
#include <stddef.h>

EventMailbox g_events = {0u, 0u, 0u};

void event_reset(void)
{
    g_events.pending = 0u;
    g_events.dropped = 0u;
    g_events.last_tick = 0u;
}

void event_post_isr(uint32_t tick)
{
    if (g_events.pending < 4u)
    {
        g_events.pending++;
        g_events.last_tick = tick;
    }
    else
    {
        if (g_events.dropped < UINT32_MAX)
        {
            g_events.dropped++;
        }
    }
}

bool event_take(EventBatch *out)
{
    if (out == NULL)
    {
        return false;
    }

    bool previous_irq = platform_irq_save_disable();

    EventBatch snapshot;

    snapshot.pending = g_events.pending;
    snapshot.dropped = g_events.dropped;
    snapshot.last_tick = g_events.last_tick;

    platform_interleave(); /* Hook 위치와 호출 횟수 유지 */

    if (snapshot.pending == 0u)
    {
        platform_irq_restore(previous_irq);
        return false;
    }

    g_events.pending = 0u;
    g_events.dropped = 0u;
    g_events.last_tick = 0u;

    platform_irq_restore(previous_irq);

    *out = snapshot;

    return true;
}