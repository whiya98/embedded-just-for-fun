#ifndef MISSION_H
#define MISSION_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define RX_LINE_CAPACITY 32u
#define TX_FRAME_CAPACITY 32u
#define FRAME_MAX_BYTES 15u

typedef struct {
    uint16_t seq;
    uint16_t value;
    uint8_t status;
} Record;

typedef struct {
    uint8_t line[RX_LINE_CAPACITY];
    size_t used;
    bool discarding;
    bool have_last;
    Record last;
} StreamRx;

typedef struct {
    uint8_t pending[TX_FRAME_CAPACITY];
    size_t length;
    size_t offset;
} StreamTx;

/* 모든 API 호출은 Main의 단일 실행 흐름에서 직렬화한다. */
void uart_configure(void);
/* line은 LF를 제외한 len byte. 실패 시 *out 보존. */
bool frame_parse(const uint8_t *line, size_t len, Record *out);
/* 성공 시 LF 포함 frame과 길이 반환. 실패 시 out과 *out_len 보존. */
bool frame_encode(const Record *record, uint8_t *out, size_t capacity,
                  size_t *out_len);
void stream_rx_init(StreamRx *rx);
/* 반환값은 이번 호출에서 수용한 완전한 frame 수. */
size_t stream_rx_feed(StreamRx *rx, const uint8_t *bytes, size_t len);
void stream_tx_init(StreamTx *tx);
/* Busy/잘못된 Record는 false이며 기존 Pending frame 보존. */
bool stream_tx_queue(StreamTx *tx, const Record *record);
/* Pending일 때 write를 최대 1회 시도한다. Busy-wait하지 않는다. */
void stream_tx_pump(StreamTx *tx);
bool stream_tx_busy(const StreamTx *tx);
#endif
