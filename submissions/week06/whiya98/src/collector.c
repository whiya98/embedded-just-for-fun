
#include "mission.h"
#include "platform.h"
#include <stddef.h>

bool collector_step(SampleReport *out)
{
    if (out == NULL)
    {
        return false;
    }

    EventBatch batch;

    if (!event_take(&batch))
    {
        return false;
    }

    SampleReport report = {0};

    report.request_tick = batch.last_tick;
    report.requests = batch.pending;
    report.coalesced = batch.pending - 1u;
    report.dropped = batch.dropped;

    report.value = platform_sensor_read();

    platform_publish(&report);

    *out = report;

    return true;
}
