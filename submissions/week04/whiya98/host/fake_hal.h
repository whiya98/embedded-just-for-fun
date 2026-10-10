#ifndef MISSION_FAKE_HAL_H
#define MISSION_FAKE_HAL_H
#include <stdbool.h>
#include <stddef.h>
#include "stm32f0xx_hal.h"

/* Fixed support. Students may read this API and use it in test_student.c. */
typedef enum {
    EVENT_HAL_INIT,
    EVENT_OSC_CONFIG,
    EVENT_CLOCK_CONFIG,
    EVENT_GPIO_CLOCK_ENABLE,
    EVENT_GPIO_WRITE,
    EVENT_GPIO_INIT,
    EVENT_GPIO_TOGGLE,
    EVENT_DELAY
} FakeEvent;
#define FAKE_TRACE_CAPACITY 128U

typedef struct {
    HAL_StatusTypeDef hal_result, osc_result, clock_result;
    FakeEvent trace[FAKE_TRACE_CAPACITY];
    size_t trace_count;
    unsigned int protocol_errors;
    bool trace_overflow, gpio_clock_enabled, gpio_initialized;
    RCC_OscInitTypeDef osc;
    RCC_ClkInitTypeDef clock;
    uint32_t flash_latency;
    GPIO_InitTypeDef gpio;
    GPIO_TypeDef *init_port, *write_port, *toggle_port;
    uint16_t write_pin, toggle_pin;
    GPIO_PinState write_state;
    uint32_t gpioa_output;
    unsigned int toggle_count, delay_count;
    uint32_t last_delay_ms;
    uint64_t delay_total_ms;
} FakeHalState;

extern FakeHalState fake_hal;
/* Resets only the adapter, not app.c's static state. */
void fake_hal_reset(void);
const char *fake_event_name(FakeEvent event);
#endif
