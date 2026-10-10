#include "mission.h"
#include "stm32f0xx_hal.h"

void board_led_init(void)
{
    /* TODO L1: GPIOA Clock Enable -> PA5 RESET -> Output PP/No Pull/Low Speed.
       HAL_GPIO_WritePin으로 초기 출력 LOW를 먼저 설정하세요. */
    
    GPIO_InitTypeDef gpio = {0};

    /* GPIOA peripheral clock enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* LED initial state: OFF */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

    /* PA5 configuration */
    gpio.Pin = GPIO_PIN_5;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(GPIOA, &gpio);
}

void board_led_toggle(void)
{
    /* TODO L2: HAL_GPIO_TogglePin으로 GPIOA의 Pin 5만 Toggle하세요. */
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
}
