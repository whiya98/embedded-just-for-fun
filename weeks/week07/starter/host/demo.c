#include "mission.h"
#include "fake_platform.h"
#include <stdio.h>
#include <string.h>
static size_t feed(StreamRx *rx, const char *text)
{
    return stream_rx_feed(rx, (const uint8_t *)text, strlen(text));
}
int main(void)
{
    fake_reset();
    uart_configure();
    printf("config baud=%lu bits=%u parity=%u stop=%u tx=%u rx=%u\n",
           (unsigned long)fake.applied.baud, fake.applied.data_bits,
           fake.applied.parity_enabled, fake.applied.stop_bits,
           fake.applied.tx_enabled, fake.applied.rx_enabled);
    StreamRx rx;
    stream_rx_init(&rx);
    size_t first = feed(&rx, "S,1,23");
    size_t second = feed(&rx, ",0\nS,2,45,1\n");
    printf("RX split+joined: accepted=%zu+%zu have_last=%u last=%u/%u/%u\n",
           first, second, rx.have_last, rx.last.seq, rx.last.value, rx.last.status);
    StreamTx tx;
    stream_tx_init(&tx);
    Record record = {65535u, 4095u, 3u};
    const size_t plan[] = {3u, 0u, 2u, 32u};
    fake_write_plan(plan, sizeof plan / sizeof plan[0]);
    bool queued = stream_tx_queue(&tx, &record);
    for (unsigned i = 0; i < 4u; ++i) stream_tx_pump(&tx);
    printf("TX queued=%u accepted=%zu calls=%zu busy=%u bytes=", queued,
           fake.wire_len, fake.write_calls, stream_tx_busy(&tx));
    for (size_t i = 0; i < fake.wire_len; ++i) {
        if (fake.wire[i] == '\n') fputs("\\n", stdout);
        else putchar((int)fake.wire[i]);
    }
    putchar('\n');
    return 0;
}
