#include "mission.h"
#include <limits.h>
#include <stddef.h>

/*
 * 05회차 한국어 해설 코드 - 시간 변환과 만료 판정
 *
 * 근거: ../../REFERENCES.md의 U3/U4/S2/S4.
 * U3 §10.5.1.3 p10-29: STM Clock은 source와 STMDIV로 결정됩니다.
 * U4 §27.4.2 p27-9: TIM0는 64-bit STM의 하위 32-bit를 읽습니다.
 * CPU 최대 Clock을 hz에 넣지 않고, 전달받은 STM 주파수를 사용합니다.
 * 정수 Tick, INT32_MAX 상한, 실패 시 값 보존은 과제의 Software 계약입니다.
 * T1~T4, P1~P4는 README.md의 근본 원인 번호에 대응합니다.
 */
bool ticks_from_ms(uint32_t hz, uint32_t ms, uint32_t *out)
{
    /* NULL을 역참조하지 않습니다. 0 Hz/0 ms도 유효한 주기가 아닙니다. */
    if (out == NULL || hz == 0U || ms == 0U) {
        return false;
    }

    /* T1: 100 MHz * 250 ms = 25,000,000,000은 32-bit를 넘습니다.
     * 저장 변수만 넓히면 늦으므로, 곱셈 전에 피연산자를 64-bit로 바꿉니다.
     * uint32_t 두 값의 최대 곱은 uint64_t에 들어갑니다.
     */
    const uint64_t product = (uint64_t)hz * ms;
    const uint64_t ticks = product / 1000U;

    /* T2/T3: 1.5 Tick 같은 값을 버림 처리하지 않습니다.
     * 정수 Tick인지 확인하고, 허용 결과인 1~INT32_MAX만 받아들입니다.
     */
    if (product % 1000U != 0U || ticks == 0U || ticks > INT32_MAX) {
        return false;
    }

    /* T4: 모든 검증이 끝난 뒤 한 번만 저장합니다.
     * 예: hz=1, ms=1, 기존 out=77이면 false와 함께 77을 보존합니다.
     */
    *out = (uint32_t)ticks;
    return true;
}

bool period_due(uint32_t now, uint32_t period, uint32_t *last)
{
    /* P4: 잘못된 주기는 거절하며 유효한 last 객체의 값은 바꾸지 않습니다.
     * INT32_MAX는 API가 허용한 period 상한이지 STM Counter의 한계가 아닙니다.
     */
    if (last == NULL || period == 0U || period > (uint32_t)INT32_MAX) {
        return false;
    }

    /* P1/P2: 마감 시각 last+period 대신 unsigned 경과 차이를 비교합니다.
     * last=UINT32_MAX-4, now=5이면 차이는 10이며, >=가 정확한 경계를 포함합니다.
     * 같은 Counter/일정한 Clock, 실제 경과 <2^32 Tick이 전제입니다.
     * 실제 경과가 INT32_MAX보다 커도 한 바퀴 미만이면 이 차이는 유효합니다.
     * Clock 변경/Reset/Debug 정지 후에는 호출 측에서 시간 기준을 재초기화합니다.
     */
    if ((uint32_t)(now - *last) >= period) {
        /* P3: 지연된 횟수를 따라잡지 않고 현재부터 새 주기를 셉니다.
         * period=10, now=35이면 last=35, 다음 만료는 45입니다.
         */
        *last = now;
        return true;
    }
    /* 미만료일 때 last를 유지해야 반복 호출로 기한이 계속 밀리지 않습니다. */
    return false;
}