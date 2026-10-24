#include "mission.h"
#include <string.h>
void stream_rx_init(StreamRx *rx)
{
    if (rx != NULL) memset(rx, 0, sizeof *rx);
}
size_t stream_rx_feed(StreamRx *rx, const uint8_t *bytes, size_t len)
{
    if (rx == NULL || (bytes == NULL && len != 0u)) return 0;
    rx->used = 0;
    for (size_t i = 0; i < len; ++i) {
        if (bytes[i] == '\n') {
            bool valid = frame_parse(rx->line, rx->used, &rx->last);
            if (valid) rx->have_last = true;
            rx->used = 0;
            return valid ? 1u : 0u;
        }
        if (rx->used >= RX_LINE_CAPACITY - 1u) rx->used = 0;
        rx->line[rx->used++] = bytes[i];
    }
    bool valid = frame_parse(rx->line, rx->used, &rx->last);
    if (valid) rx->have_last = true;
    return valid ? 1u : 0u;
}
