#include "mission.h"
#include "fake_hal.h"
#include <assert.h>

void run_student_tests(void)
{
    /*
     * Case 1: Flash 상한 경계 검증
     * 요구사항: Flash 영역은 0x08000000 ~ 0x0800FFFF이다.
     * 입력: Flash 마지막 주소와 바로 다음 주소
     * 예상 결과: 마지막 주소는 MEMORY_FLASH,
     *            다음 주소는 MEMORY_UNKNOWN
     * 검증 이유: 메모리 영역의 상한 경계가 정확히 처리되는지 확인한다.
     */
    assert(memory_region(0x0800FFFFu) == MEMORY_FLASH);
    assert(memory_region(0x08010000u) == MEMORY_UNKNOWN);

    /*
     * Case 2: HAL 초기화 실패 시 Application 동작 차단 검증
     * 요구사항: HAL_Init 실패 시 app_init은 false를 반환하고
     *           이후 app_step은 LED를 Toggle하거나 Delay하지 않아야 한다.
     * 입력: HAL_Init의 반환값으로 HAL_ERROR 주입
     * 예상 결과: app_init() == false,
     *            toggle_count == 0, delay_count == 0
     * 검증 이유: 초기화 실패 후 반복 동작이 수행되지 않는지 확인한다.
     */
    fake_hal_reset();
    fake_hal.hal_result = HAL_ERROR;

    assert(app_init() == false);

    app_step();

    assert(fake_hal.toggle_count == 0U);
    assert(fake_hal.delay_count == 0U);
}