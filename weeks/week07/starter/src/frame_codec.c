#include "mission.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

static bool number(const uint8_t *line, size_t len, size_t *at, uint32_t *value)
{
    while (*at < len && (line[*at] == ' ' || line[*at] == '\t')) ++*at;
    if (*at < len && line[*at] == '+') ++*at;
    size_t begin = *at;
    uint32_t result = 0;
    while (*at < len && line[*at] >= '0' && line[*at] <= '9') {
        uint32_t digit = (uint32_t)(line[*at] - '0');
        result = result > (UINT32_MAX - digit) / 10u
            ? UINT32_MAX : result * 10u + digit;
        ++*at;
    }
    if (*at == begin) return false;
    *value = result;
    return true;
}

bool frame_parse(const uint8_t *line, size_t len, Record *out)
{
    if (line == NULL || out == NULL || len < 2u || len >= RX_LINE_CAPACITY)
        return false;
    if (line[0] != 'S' || line[1] != ',') return false;
    size_t at = 2u;
    uint32_t value;
    if (!number(line, len, &at, &value)) return false;
    out->seq = (uint16_t)value;
    if (at >= len || line[at++] != ',') return false;
    if (!number(line, len, &at, &value)) return false;
    out->value = (uint16_t)value;
    if (at >= len || line[at++] != ',') return false;
    if (!number(line, len, &at, &value)) return false;
    out->status = (uint8_t)value;
    return true;
}

bool frame_encode(const Record *record, uint8_t *out, size_t capacity,
                  size_t *out_len)
{
    if (record == NULL || out == NULL || out_len == NULL ||
        record->value > 4095u || record->status > 3u) return false;
    char scratch[TX_FRAME_CAPACITY];
    int count = snprintf(scratch, sizeof scratch, "S,%" PRIu16 ",%" PRIu16 ",%u\n",
                         record->seq, record->value, (unsigned)record->status);
    if (count < 0 || (size_t)count >= sizeof scratch ||
        (size_t)count > FRAME_MAX_BYTES || (size_t)count > capacity) return false;
    memcpy(out, scratch, (size_t)count);
    *out_len = (size_t)count;
    return true;
}
