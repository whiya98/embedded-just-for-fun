
#include "mission.h"
#include "platform.h"

void timer_isr(void)
{
    // 1. Timer 서비스 (다음 Compare 처리 경계)
    platform_timer_service();

    // 2. 현재 Timer Tick 읽기
    uint32_t tick = platform_timer_now();

    // 3. Mailbox에 수집 요청 기록
    event_post_isr(tick);
}