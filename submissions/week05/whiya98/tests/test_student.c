#include "mission.h"
#include "fake_platform.h"
#include <assert.h>

void run_student_tests(void)
{
    /*
     * Case 1: 수집 주기 만료 시 ENABLE이 비활성화된 경우
     *
     * 요구사항:
     * ENABLE이 비활성 상태여도 수집 주기의 기준 시각은 갱신한다.
     * 이후 다시 활성화되어도 비활성 기간의 Sample을 몰아서 수집하지 않는다.
     *
     * 조건:
     * fSTM = 1000 Hz → 10 ms = 10 ticks
     * 초기 시각 = 0
     * 10 tick: disabled
     * 15 tick: enabled
     * 20 tick: enabled
     *
     * 예상 결과:
     * 10 tick에서는 sample을 읽지 않지만 last_sample은 10으로 갱신
     * 15 tick에서는 아직 5 tick만 지났으므로 sample 없음
     * 20 tick에서 처음 sample 1회 수행
     */
    CollectorState state = {0};

    fake_platform_reset();
    assert(collector_init(&state, 1000U, 0U));

    fake_set_enabled(false);
    collector_step(&state, 10U);

    assert(state.sample_count == 0U);
    assert(state.last_sample == 10U);

    fake_set_enabled(true);
    collector_step(&state, 15U);

    assert(state.sample_count == 0U);

    collector_step(&state, 20U);

    assert(state.sample_count == 1U);
    assert(state.last_sample == 20U);


    /*
     * Case 2: 초기화 후 첫 호출이 250 ms 늦게 들어온 경우
     *
     * 요구사항:
     * 늦게 호출되더라도 한 번의 collector_step()에서
     * Sample과 LED 작업을 각각 최대 1회만 수행한다.
     *
     * 조건:
     * fSTM = 1000 Hz
     * Sample period = 10 ticks
     * LED period = 250 ticks
     * 초기 시각 = 0
     * 첫 collector_step() = 250
     *
     * 예상 결과:
     * 지나간 10 ms Sample을 25회 Catch-up하지 않고 1회만 수집
     * LED도 1회만 Toggle
     * 두 기준 시각 모두 현재 now=250으로 갱신
     */
    fake_platform_reset();
    state = (CollectorState){0};

    assert(collector_init(&state, 1000U, 0U));

    fake_set_enabled(true);
    collector_step(&state, 250U);

    assert(state.sample_count == 1U);
    assert(fake.sensor_calls == 1U);
    assert(fake.toggle_calls == 1U);

    assert(state.last_sample == 250U);
    assert(state.last_led == 250U);

    assert(fake.delay_calls == 0U);
}