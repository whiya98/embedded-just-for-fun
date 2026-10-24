#include "mission.h"
#include "fake_platform.h"
#include "test_support.h"
#include <stdio.h>
#include <string.h>

static unsigned checks;
static unsigned failures;
static const char *scenario = "start";
void test_check(bool result, const char *expression, const char *file, int line)
{
    ++checks;
    if (!result) {
        ++failures;
        fputs("FAIL [", stdout);
        for (const unsigned char *p = (const unsigned char *)scenario; *p; ++p) {
            if (*p == '\n') fputs("\\n", stdout);
            else if (*p == '\r') fputs("\\r", stdout);
            else if (*p == '\t') fputs("\\t", stdout);
            else putchar(*p);
        }
        printf("] %s:%d: %s\n", file, line, expression);
    }
}
static bool same_record(Record a, Record b)
{
    return a.seq == b.seq && a.value == b.value && a.status == b.status;
}
static size_t feed_text(StreamRx *rx, const char *text)
{
    return stream_rx_feed(rx, (const uint8_t *)text, strlen(text));
}
static bool parse_text(const char *text, Record *out)
{
    return frame_parse((const uint8_t *)text, strlen(text), out);
}
static bool same_tx(const StreamTx *a, const StreamTx *b)
{
    return a->length == b->length && a->offset == b->offset &&
        memcmp(a->pending, b->pending, sizeof a->pending) == 0;
}
static void test_config(void)
{
    scenario = "UART-config";
    fake_reset();
    uart_configure();
    CHECK(fake.configure_calls == 1u);
    CHECK(fake.applied.baud == 115200u);
    CHECK(fake.applied.data_bits == 8u);
    CHECK(!fake.applied.parity_enabled);
    CHECK(fake.applied.stop_bits == 1u);
    CHECK(fake.applied.tx_enabled);
    CHECK(fake.applied.rx_enabled);
}
static void test_parse_valid(void)
{
    scenario = "valid-records";
    const char *text[] = {"S,0,0,0", "S,65535,4095,3", "S,1,9,2", "S,42,2048,1"};
    const Record expected[] = {{0,0,0}, {65535,4095,3}, {1,9,2}, {42,2048,1}};
    for (size_t i = 0; i < sizeof text / sizeof text[0]; ++i) {
        Record out = {7,8,1};
        CHECK(parse_text(text[i], &out));
        CHECK(same_record(out, expected[i]));
    }
}
static void test_parse_invalid(void)
{
    const char *invalid[] = {
        "S,65536,1,1", "S,1,4096,1", "S,1,2,4", "S,999999999999999999999,1,0",
        "S,01,2,3", "S,1,02,3", "S,1,2,03", "S,+1,2,3", "S, 1,2,3",
        "S,1,\t2,3", "S,1,2,+3", "S,1,2,3 ", "S,1,2,3\r", "S,1,2,3x",
        "S,1,2,3,4", "S,1,2,3\n", "S,-1,2,3", "S,1,-2,3", "S,1,2,-3",
        "S,,2,3", "S,1,,3", "S,1,2,", "S,1,2,x", "S,1x,2,3",
        "s,1,2,3", "X,1,2,3", "S,1,2", "S,1", "S,", "S", "", ",,,",
        "S,1,2,3S,4,5,0", " S,1,2,3"
    };
    const Record before = {17,18,2};
    for (size_t i = 0; i < sizeof invalid / sizeof invalid[0]; ++i) {
        scenario = invalid[i];
        Record out = before;
        CHECK(!parse_text(invalid[i], &out));
        CHECK(same_record(out, before));
    }
    scenario = "NUL/non-ASCII/bounds";
    const uint8_t nul[] = {'S',',','1',',','2',',','3',0,'x'};
    const uint8_t high[] = {'S',',','1',',','2',',','3',0xff};
    Record out = before;
    CHECK(!frame_parse(nul, sizeof nul, &out));
    CHECK(same_record(out, before));
    CHECK(!frame_parse(high, sizeof high, &out));
    CHECK(same_record(out, before));
    uint8_t longline[32];
    memset(longline, '1', sizeof longline);
    CHECK(!frame_parse(longline, sizeof longline, &out));
    CHECK(same_record(out, before));
    CHECK(!frame_parse(NULL, 0, &out));
    CHECK(same_record(out, before));
    CHECK(!frame_parse((const uint8_t *)"S,1,2,3", 7, NULL));
}
static void test_encode(void)
{
    scenario = "encode-boundaries";
    const Record records[] = {{0,0,0}, {65535,4095,3}, {42,9,1}};
    const char *wire[] = {"S,0,0,0\n", "S,65535,4095,3\n", "S,42,9,1\n"};
    for (size_t i = 0; i < sizeof records / sizeof records[0]; ++i) {
        uint8_t out[TX_FRAME_CAPACITY];
        memset(out, 0xa5, sizeof out);
        size_t length = 999;
        size_t expected = strlen(wire[i]);
        CHECK(frame_encode(&records[i], out, expected, &length));
        CHECK(length == expected);
        CHECK(memcmp(out, wire[i], expected) == 0);
        CHECK(out[expected] == 0xa5u);
        Record parsed = {4,5,2};
        CHECK(frame_parse(out, length - 1u, &parsed));
        CHECK(same_record(parsed, records[i]));
        memset(out, 0xa5, sizeof out);
        uint8_t before[TX_FRAME_CAPACITY];
        memcpy(before, out, sizeof before);
        length = 999;
        CHECK(!frame_encode(&records[i], out, expected - 1u, &length));
        CHECK(length == 999u);
        CHECK(memcmp(out, before, sizeof out) == 0);
    }
    uint8_t out[TX_FRAME_CAPACITY];
    memset(out, 0xa5, sizeof out);
    uint8_t before[TX_FRAME_CAPACITY];
    memcpy(before, out, sizeof before);
    const Record invalid[] = {{1,4096,0}, {1,2,4}};
    size_t length = 999;
    for (size_t i = 0; i < sizeof invalid / sizeof invalid[0]; ++i) {
        CHECK(!frame_encode(&invalid[i], out, sizeof out, &length));
        CHECK(length == 999u);
        CHECK(memcmp(out, before, sizeof out) == 0);
    }
    const Record valid = {1,2,3};
    CHECK(!frame_encode(NULL, out, sizeof out, &length));
    CHECK(!frame_encode(&valid, NULL, sizeof out, &length));
    CHECK(!frame_encode(&valid, out, sizeof out, NULL));
    CHECK(length == 999u);
    CHECK(memcmp(out, before, sizeof out) == 0);
    CHECK(FRAME_MAX_BYTES == strlen("S,65535,4095,3\n"));
}
static void test_rx_complete_and_pending(void)
{
    scenario = "RX-complete-and-pending";
    StreamRx rx;
    stream_rx_init(&rx);
    CHECK(!rx.have_last);
    CHECK(feed_text(&rx, "S,1,2,3\n") == 1u);
    CHECK(rx.have_last);
    CHECK(same_record(rx.last, (Record){1,2,3}));
    CHECK(feed_text(&rx, "S,9,8,1") == 0u);
    CHECK(same_record(rx.last, (Record){1,2,3}));
    CHECK(stream_rx_feed(&rx, NULL, 0) == 0u);
    CHECK(feed_text(&rx, "\n") == 1u);
    CHECK(same_record(rx.last, (Record){9,8,1}));
    CHECK(rx.used == 0u);
    CHECK(!rx.discarding);
    CHECK(feed_text(&rx, "S,99,8,x\n") == 0u);
    CHECK(same_record(rx.last, (Record){9,8,1}));
    CHECK(rx.have_last);
    stream_rx_init(&rx);
    CHECK(!rx.have_last);
    CHECK(rx.used == 0u);
    CHECK(!rx.discarding);
}
static void test_rx_all_splits(void)
{
    const char *text = "S,65535,4095,3\n";
    size_t len = strlen(text);
    for (size_t split = 0; split <= len; ++split) {
        scenario = "RX-every-two-chunk-split";
        StreamRx rx;
        stream_rx_init(&rx);
        size_t first = stream_rx_feed(&rx, (const uint8_t *)text, split);
        CHECK(first == (split == len ? 1u : 0u));
        CHECK(rx.have_last == (split == len));
        CHECK(stream_rx_feed(&rx, NULL, 0) == 0u);
        size_t second = stream_rx_feed(&rx, (const uint8_t *)text + split, len - split);
        CHECK(first + second == 1u);
        CHECK(rx.have_last);
        CHECK(same_record(rx.last, (Record){65535,4095,3}));
    }
    scenario = "RX-one-byte-chunks";
    StreamRx rx;
    stream_rx_init(&rx);
    for (size_t i = 0; i < len; ++i) {
        size_t count = stream_rx_feed(&rx, (const uint8_t *)text + i, 1u);
        CHECK(count == (i + 1u == len ? 1u : 0u));
    }
    CHECK(same_record(rx.last, (Record){65535,4095,3}));
}
static void test_rx_joined_and_recovery(void)
{
    scenario = "RX-joined-valid-and-invalid";
    StreamRx rx;
    stream_rx_init(&rx);
    CHECK(feed_text(&rx, "S,1,2,3\nS,4,5,0\n") == 2u);
    CHECK(same_record(rx.last, (Record){4,5,0}));
    CHECK(feed_text(&rx, "bad\nS,6,7,1\nS,8,9,2\n") == 2u);
    CHECK(same_record(rx.last, (Record){8,9,2}));
    CHECK(feed_text(&rx, "\n\nS,10,11,3\n\n") == 1u);
    CHECK(same_record(rx.last, (Record){10,11,3}));
    const uint8_t binary[] = {'S',',','1',',','2',',','3',0,'\n',
                              'S',',','4',',','5',',','0','\n'};
    CHECK(stream_rx_feed(&rx, binary, sizeof binary) == 1u);
    CHECK(same_record(rx.last, (Record){4,5,0}));
    CHECK(feed_text(&rx, "S,4,5,1\r\nS,3,2,0\n") == 1u);
    CHECK(same_record(rx.last, (Record){3,2,0}));
}
static void test_rx_overlong(void)
{
    uint8_t text[96];
    memset(text, 'X', 31);
    const char *tail = "S,1,2,3\nS,9,10,2\n";
    memcpy(text + 31u, tail, strlen(tail));
    size_t len = 31u + strlen(tail);
    /* 첫 line의 suffix는 문법이 맞아도 독립 frame이 아니다. */
    for (size_t split = 0; split <= len; ++split) {
        scenario = "RX-overlong-through-LF-recovery";
        StreamRx rx;
        stream_rx_init(&rx);
        size_t a = stream_rx_feed(&rx, text, split);
        size_t b = stream_rx_feed(&rx, text + split, len - split);
        CHECK(a + b == 1u);
        CHECK(rx.have_last);
        CHECK(same_record(rx.last, (Record){9,10,2}));
        CHECK(rx.used == 0u);
        CHECK(!rx.discarding);
    }
    scenario = "RX-line-capacity-state";
    StreamRx rx;
    stream_rx_init(&rx);
    uint8_t bytes[33];
    memset(bytes, 'X', sizeof bytes);
    CHECK(stream_rx_feed(&rx, bytes, 31u) == 0u);
    CHECK(rx.used <= RX_LINE_CAPACITY - 1u);
    CHECK(!rx.discarding);
    CHECK(stream_rx_feed(&rx, bytes, 1u) == 0u);
    CHECK(rx.discarding);
    CHECK(feed_text(&rx, "S,1,2,3") == 0u);
    CHECK(!rx.have_last);
    CHECK(feed_text(&rx, "\nS,7,8,1\n") == 1u);
    CHECK(same_record(rx.last, (Record){7,8,1}));
}
static void test_null_and_idle(void)
{
    scenario = "NULL-and-idle";
    StreamRx rx;
    stream_rx_init(&rx);
    CHECK(feed_text(&rx, "S,7,") == 0u);
    size_t used = rx.used;
    CHECK(stream_rx_feed(&rx, NULL, 1u) == 0u);
    CHECK(rx.used == used);
    CHECK(stream_rx_feed(NULL, (const uint8_t *)"x", 1u) == 0u);
    stream_rx_init(NULL);
    StreamTx tx;
    stream_tx_init(&tx);
    fake_reset();
    CHECK(!stream_tx_busy(&tx));
    CHECK(!stream_tx_busy(NULL));
    stream_tx_pump(&tx);
    stream_tx_pump(NULL);
    CHECK(fake.write_calls == 0u);
    CHECK(!stream_tx_queue(NULL, &(Record){1,2,3}));
    CHECK(!stream_tx_queue(&tx, NULL));
    CHECK(!stream_tx_busy(&tx));
    stream_tx_init(NULL);
}
static void test_tx_normal(void)
{
    scenario = "TX-normal-and-repeated-pump";
    StreamTx tx;
    stream_tx_init(&tx);
    fake_reset();
    CHECK(stream_tx_queue(&tx, &(Record){65535,4095,3}));
    CHECK(stream_tx_busy(&tx));
    CHECK(fake.write_calls == 0u);
    stream_tx_pump(&tx);
    CHECK(!stream_tx_busy(&tx));
    CHECK(fake.write_calls == 1u);
    CHECK(fake.wire_len == FRAME_MAX_BYTES);
    CHECK(memcmp(fake.wire, "S,65535,4095,3\n", FRAME_MAX_BYTES) == 0);
    stream_tx_pump(&tx);
    CHECK(fake.write_calls == 1u);
    CHECK(fake.wire_len == FRAME_MAX_BYTES);
    CHECK(stream_tx_queue(&tx, &(Record){0,0,0}));
    stream_tx_pump(&tx);
    CHECK(fake.wire_len == FRAME_MAX_BYTES + 8u);
    CHECK(memcmp(fake.wire + FRAME_MAX_BYTES, "S,0,0,0\n", 8u) == 0);
}
static void test_tx_partial(void)
{
    scenario = "TX-partial-zero-and-order";
    StreamTx tx;
    stream_tx_init(&tx);
    fake_reset();
    const size_t plan[] = {3u,0u,2u,32u};
    fake_write_plan(plan, sizeof plan / sizeof plan[0]);
    CHECK(stream_tx_queue(&tx, &(Record){65535,4095,3}));
    const size_t expected_wire[] = {3u,3u,5u,15u};
    for (size_t i = 0; i < 4u; ++i) {
        size_t before_calls = fake.write_calls;
        stream_tx_pump(&tx);
        CHECK(fake.write_calls == before_calls + 1u);
        CHECK(fake.wire_len == expected_wire[i]);
        CHECK(memcmp(fake.wire, "S,65535,4095,3\n", fake.wire_len) == 0);
        CHECK(stream_tx_busy(&tx) == (i != 3u));
        if (i < 3u) CHECK(tx.offset == expected_wire[i]);
    }
    CHECK(fake.largest_requested == 15u);
    stream_tx_pump(&tx);
    CHECK(fake.write_calls == 4u);
    CHECK(fake.wire_len == 15u);
}
static void test_tx_busy_and_invalid(void)
{
    scenario = "TX-busy-whole-frame-preservation";
    StreamTx tx;
    stream_tx_init(&tx);
    fake_reset();
    CHECK(stream_tx_queue(&tx, &(Record){1,2,3}));
    StreamTx before = tx;
    CHECK(!stream_tx_queue(&tx, &(Record){4,5,0}));
    CHECK(same_tx(&tx, &before));
    const size_t plan[] = {2u,0u,32u};
    fake_write_plan(plan, sizeof plan / sizeof plan[0]);
    stream_tx_pump(&tx);
    before = tx;
    CHECK(!stream_tx_queue(&tx, &(Record){6,7,1}));
    CHECK(same_tx(&tx, &before));
    CHECK(!stream_tx_queue(&tx, &(Record){8,4096,1}));
    CHECK(same_tx(&tx, &before));
    stream_tx_pump(&tx);
    CHECK(same_tx(&tx, &before));
    stream_tx_pump(&tx);
    CHECK(!stream_tx_busy(&tx));
    CHECK(fake.wire_len == 8u);
    CHECK(memcmp(fake.wire, "S,1,2,3\n", 8u) == 0);
    before = tx;
    CHECK(!stream_tx_queue(&tx, &(Record){1,2,4}));
    CHECK(same_tx(&tx, &before));
    CHECK(!stream_tx_queue(&tx, NULL));
    CHECK(same_tx(&tx, &before));
}
int main(void)
{
    test_config();
    test_parse_valid();
    test_parse_invalid();
    test_encode();
    test_rx_complete_and_pending();
    test_rx_all_splits();
    test_rx_joined_and_recovery();
    test_rx_overlong();
    test_null_and_idle();
    test_tx_normal();
    test_tx_partial();
    test_tx_busy_and_invalid();
    scenario = "additional-cases";
    test_student_cases();
    printf("checks=%u failures=%u\n", checks, failures);
    return failures == 0u ? 0 : 1;
}
