#include "mission.h"
#include "platform.h"

/*
 * 05회차 한국어 해설 코드 - GPIO 초기 상태, 입력 의미, Bit 보존
 *
 * 근거: ../../REFERENCES.md의 D1/B1/U3/S3.
 * D1 Figure 2-2 p13, Table 2-25 p160: BGA292의 P13.0/P00.0 Pin 확인.
 * B1 §3.15 p3-6: 참조 Board D107은 P13.0에 연결된 Active LOW LED입니다.
 * U3 §14.4.2/§14.4.8: IOCR Mode 설정과 OMR의 Pin별 출력 변경을 확인합니다.
 * platform_*는 Host 대역입니다. PORT_LED/PORT_ENABLE은 실제 MMIO 주소가 아닙니다.
 */
void board_io_init(void)
{
    /* G1/G2: Active LOW이므로 HIGH가 OFF입니다.
     * write(port, set, clear)의 두 번째 인자는 HIGH로 만들 Mask입니다.
     * LOW Latch가 출력되지 않도록 HIGH를 준비한 뒤 Output으로 전환합니다.
     */
    platform_port_write(PORT_LED, LED_MASK, 0U);
    platform_port_output_pushpull(PORT_LED, LED_MASK);

    /* G3: 과제에서 정한 외부 ENABLE 접점은 Pull-up + GND 연결입니다.
     * 열림=HIGH, 접점 연결=LOW이며 RESET/WAKE Button을 대신 쓰지 않습니다.
     * Host에서는 전압을 측정하지 않고 설정 요청과 논리 입력만 검증합니다.
     */
    platform_port_input(PORT_ENABLE, ENABLE_MASK, true);
}

bool board_collection_enabled(void)
{
    /* G4: read의 true는 HIGH입니다. 수집 허용은 LOW이므로 논리값을 뒤집습니다. */
    return !platform_port_read(PORT_ENABLE, ENABLE_MASK);
}

void board_led_toggle(void)
{
    /* G5: LED_MASK만 변경합니다. UINT32_MAX는 다른 출력까지 뒤집습니다.
     * 예: Port=0xA4, Mask=0x01이면 결과는 0xA5여야 합니다.
     */
    platform_port_toggle(PORT_LED, LED_MASK);
}