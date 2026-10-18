#include "mission.h"
#include "platform.h"
#include <stddef.h>

/*
 * 05회차 한국어 해설 코드 - 서로 독립적인 Polling 작업
 *
 * 10 ms 수집/250 ms Toggle/지연 시 한 번 처리/실패 후 I/O 금지는 제품 요구입니다.
 * Datasheet가 이 Software 정책을 정하는 것은 아닙니다.
 * Single-core 순차 호출, 안정된 입력, 모의 센서 값 0~4095를 전제합니다.
 * I1~I3, C1~C5는 README.md의 근본 원인 번호에 대응합니다.
 */
bool collector_init(CollectorState *state, uint32_t stm_hz, uint32_t now)
{
    if (state == NULL) {
        return false;
    }

    /* I2: 새 시도에서는 이전 ready, 횟수, 값, 기준 시각을 모두 초기화합니다.
     * 이후 실패해도 과거의 ready=true가 남아 반복 동작을 허용하지 않습니다.
     * {0}은 각 Field를 0/false로 초기화하며 Padding Byte의 값에 의존하지 않습니다.
     */
    *state = (CollectorState){0};
    uint32_t sample_period;
    uint32_t led_period;

    /* I1/I3: 고정 100 MHz가 아니라 전달받은 stm_hz로 두 주기를 검증합니다.
     * 결과를 지역변수에 받으므로 한쪽만 성공해도 state는 초기 상태입니다.
     * 두 계산이 성공하기 전까지 GPIO를 호출하지 않습니다.
     */
    if (!ticks_from_ms(stm_hz, 10U, &sample_period) ||
        !ticks_from_ms(stm_hz, 250U, &led_period)) {
        return false;
    }
    state->sample_period = sample_period;
    state->led_period = led_period;

    /* 두 작업을 현재 시각에서 시작합니다. 같은 now의 첫 Step에서는 만료되지 않습니다. */
    state->last_sample = now;
    state->last_led = now;
    board_io_init();

    /* 모든 준비가 끝난 뒤 ready를 설정합니다. 실패 후 기존 Hardware의
     * 전기적 상태까지 되돌리는 Rollback은 이 과제의 계약에 포함되지 않습니다.
     */
    state->ready = true;
    return true;
}

void collector_step(CollectorState *state, uint32_t now)
{
    /* C1: 준비 전/초기화 실패 후에는 ENABLE 읽기를 포함한 모든 I/O를 막습니다. */
    if (state == NULL || !state->ready) {
        return;
    }

    /* 정상 호출마다 한 번만 읽어 이번 Step에서 사용할 입력을 확정합니다. */
    const bool enabled = board_collection_enabled();

    /* C2: 만료 판정을 enabled보다 먼저 수행합니다.
     * 비활성 상태라도 period_due가 last_sample을 now로 갱신해야 합니다.
     * 10 ms HIGH -> 15 ms LOW에서는 수집하지 않고, 20 ms에 수집합니다.
     * enabled && period_due(...)로 바꾸면 Short-circuit 때문에 이 계약이 깨집니다.
     */
    if (period_due(now, state->sample_period, &state->last_sample)) {
        if (enabled) {
            state->last_value = platform_sensor_read();

            /* C4: 실제로 읽었을 때만 1 증가시킵니다. 누락된 과거 값을 세지 않습니다.
             * 한 초기화 사이 수집 횟수는 2^32회 미만이라는 과제 전제가 있습니다.
             */
            ++state->sample_count;
        }
    }

    /* C3: LED는 수집 분기 밖에서 자기 주기를 확인하므로 비활성 중에도 동작합니다. */
    if (period_due(now, state->led_period, &state->last_led)) {
        board_led_toggle();
    }

    /* C5: Delay/Sleep/Busy-wait/UART를 추가하지 않고 호출 측으로 돌아갑니다.
     * 각 작업은 최대 한 번만 실행합니다. 실제 Sensor 함수가 Blocking이면
     * Loop도 지연되므로 Board 적용 시에는 그 함수의 처리 시간도 확인해야 합니다.
     */
}