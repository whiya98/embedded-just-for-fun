#include "mission.h"
#include "stm32f0xx_hal.h"

bool board_clock_init(void)
{
    /* TODO C1: HSI 8 MHz / 2 * 12, SYSCLK/HCLK/PCLK1 48 MHz.
       RCC_OscInitTypeDef, RCC_ClkInitTypeDef를 초기화한 뒤 HAL에 전달하세요.
       HSI ON + 기본 Calibration, PLL ON, PREDIV DIV1을 명시하세요.
       OscConfig -> ClockConfig(FLASH_LATENCY_1). 실패 즉시 false. */
    
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};

    /* 1. HSI + PLL 설정 */
    osc.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    osc.HSIState = RCC_HSI_ON;
    osc.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;

    osc.PLL.PLLState = RCC_PLL_ON;
    osc.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    osc.PLL.PREDIV = RCC_PREDIV_DIV1;
    osc.PLL.PLLMUL = RCC_PLL_MUL12;

    if (HAL_RCC_OscConfig(&osc) != HAL_OK)
    {
        return false;
    }

    /* 2. SYSCLK / HCLK / PCLK1 설정 */
    clk.ClockType =
        RCC_CLOCKTYPE_SYSCLK |
        RCC_CLOCKTYPE_HCLK |
        RCC_CLOCKTYPE_PCLK1;

    clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_1) != HAL_OK)
    {
        return false;
    }

    return true;
}
