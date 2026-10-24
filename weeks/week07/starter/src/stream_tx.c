#include "mission.h"
#include "platform.h"
#include <string.h>
void stream_tx_init(StreamTx *tx)
{
    if (tx != NULL) memset(tx, 0, sizeof *tx);
}
bool stream_tx_busy(const StreamTx *tx)
{
    return tx != NULL && tx->length != 0u;
}
bool stream_tx_queue(StreamTx *tx, const Record *record)
{
    if (tx == NULL) return false;
    size_t len = 0;
    if (!frame_encode(record, tx->pending, sizeof tx->pending, &len)) return false;
    tx->length = len;
    tx->offset = 0;
    return true;
}
void stream_tx_pump(StreamTx *tx)
{
    if (!stream_tx_busy(tx)) return;
    (void)platform_uart_write(tx->pending + tx->offset, tx->length - tx->offset);
    tx->length = 0;
    tx->offset = 0;
}
