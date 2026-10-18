#include "mission.h"
#include "stm32f0xx_hal.h"

static bool initialized = false;

bool app_init(void)
{
    initialized = false;

    if (HAL_Init() != HAL_OK)
    {
        return false;
    }

    if (!board_clock_init())
    {
        return false;
    }

    board_led_init();

    initialized = true;

    return true;
}

void app_step(void)
{
    if (!initialized)
    {
        return;
    }

    board_led_toggle();
    HAL_Delay(250);
}