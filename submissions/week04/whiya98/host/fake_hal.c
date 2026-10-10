#include "fake_hal.h"
#include <string.h>

GPIO_TypeDef mission_host_gpioa;
FakeHalState fake_hal;

static void record(FakeEvent event)
{
    if (fake_hal.trace_count < FAKE_TRACE_CAPACITY) {
        fake_hal.trace[fake_hal.trace_count++] = event;
    } else {
        fake_hal.trace_overflow = true;
    }
}

void fake_hal_reset(void)
{
    memset(&fake_hal, 0, sizeof fake_hal);
    /* Begin with every bit HIGH so tests can detect unrelated-bit changes. */
    fake_hal.gpioa_output = 0xFFFFU;
    fake_hal.hal_result = HAL_OK;
    fake_hal.osc_result = HAL_OK;
    fake_hal.clock_result = HAL_OK;
}

const char *fake_event_name(FakeEvent event)
{
    static const char *const names[] = {
        "HAL_Init", "HAL_RCC_OscConfig", "HAL_RCC_ClockConfig",
        "GPIOA Clock Enable", "HAL_GPIO_WritePin", "HAL_GPIO_Init",
        "HAL_GPIO_TogglePin", "HAL_Delay"
    };
    if ((unsigned int)event >= sizeof names / sizeof names[0]) {
        return "unknown";
    }
    return names[event];
}

HAL_StatusTypeDef HAL_Init(void)
{
    record(EVENT_HAL_INIT);
    return fake_hal.hal_result;
}

HAL_StatusTypeDef HAL_RCC_OscConfig(RCC_OscInitTypeDef *config)
{
    record(EVENT_OSC_CONFIG);
    if (config == NULL) {
        ++fake_hal.protocol_errors;
        return HAL_ERROR;
    }
    fake_hal.osc = *config;
    return fake_hal.osc_result;
}

HAL_StatusTypeDef HAL_RCC_ClockConfig(RCC_ClkInitTypeDef *config,
                                    uint32_t latency)
{
    record(EVENT_CLOCK_CONFIG);
    if (config == NULL) {
        ++fake_hal.protocol_errors;
        return HAL_ERROR;
    }
    fake_hal.clock = *config;
    fake_hal.flash_latency = latency;
    return fake_hal.clock_result;
}

void mission_host_gpioa_clock_enable(void)
{
    record(EVENT_GPIO_CLOCK_ENABLE);
    fake_hal.gpio_clock_enabled = true;
}

void HAL_GPIO_WritePin(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState state)
{
    record(EVENT_GPIO_WRITE);
    fake_hal.write_port = port;
    fake_hal.write_pin = pin;
    fake_hal.write_state = state;
    if (port != GPIOA || !fake_hal.gpio_clock_enabled) {
        ++fake_hal.protocol_errors;
        return;
    }
    if (state == GPIO_PIN_RESET) {
        fake_hal.gpioa_output &= ~(uint32_t)pin;
    } else if (state == GPIO_PIN_SET) {
        fake_hal.gpioa_output |= pin;
    } else {
        ++fake_hal.protocol_errors;
    }
}

void HAL_GPIO_Init(GPIO_TypeDef *port, GPIO_InitTypeDef *config)
{
    record(EVENT_GPIO_INIT);
    fake_hal.init_port = port;
    if (config == NULL) {
        ++fake_hal.protocol_errors;
        return;
    }
    fake_hal.gpio = *config;
    if (port != GPIOA || !fake_hal.gpio_clock_enabled) {
        ++fake_hal.protocol_errors;
        return;
    }
    fake_hal.gpio_initialized = true;
}

void HAL_GPIO_TogglePin(GPIO_TypeDef *port, uint16_t pin)
{
    record(EVENT_GPIO_TOGGLE);
    fake_hal.toggle_port = port;
    fake_hal.toggle_pin = pin;
    ++fake_hal.toggle_count;
    if (port != GPIOA || !fake_hal.gpio_clock_enabled ||
        !fake_hal.gpio_initialized) {
        ++fake_hal.protocol_errors;
        return;
    }
    fake_hal.gpioa_output ^= pin;
}

void HAL_Delay(uint32_t milliseconds)
{
    record(EVENT_DELAY);
    ++fake_hal.delay_count;
    fake_hal.last_delay_ms = milliseconds;
    fake_hal.delay_total_ms += milliseconds;
    /* No Host sleep: this adapter records the requested time only. */
}
