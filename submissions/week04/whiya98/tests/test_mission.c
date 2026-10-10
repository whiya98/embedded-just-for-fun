/* Fixed public tests: do not modify this file to make the assignment pass. */
#include "mission.h"
#include "fake_hal.h"
#include <inttypes.h>
#include <stdio.h>

void run_student_tests(void);
static unsigned int checks;
static unsigned int failures;
static const char *section;

#define CHECK(condition) do { \
    ++checks; \
    if (!(condition)) { \
        ++failures; \
        fprintf(stderr, "FAIL [%s] line %d: %s\n", section, __LINE__, #condition); \
    } \
} while (0)

static void expect_trace(const FakeEvent *expected, size_t length)
{
    size_t i;
    CHECK(fake_hal.trace_count == length);
    CHECK(!fake_hal.trace_overflow);
    for (i = 0; i < length && i < fake_hal.trace_count; ++i) {
        if (fake_hal.trace[i] != expected[i]) {
            fprintf(stderr, "  call %zu: expected %s, got %s\n", i + 1U,
                    fake_event_name(expected[i]), fake_event_name(fake_hal.trace[i]));
        }
        CHECK(fake_hal.trace[i] == expected[i]);
    }
    CHECK(fake_hal.protocol_errors == 0U);
}

static void test_before_init(void)
{
    section = "A2 before initialization";
    fake_hal_reset();
    app_step();
    app_step();
    CHECK(fake_hal.trace_count == 0U);
    CHECK(fake_hal.toggle_count == 0U);
    CHECK(fake_hal.delay_count == 0U);
}

static void test_memory(void)
{
    static const struct { uint32_t address; MemoryRegion expected; } cases[] = {
        { 0x00000000U, MEMORY_UNKNOWN }, /* Boot alias is outside this API's contract. */
        { 0x07FFFFFFU, MEMORY_UNKNOWN },
        { 0x08000000U, MEMORY_FLASH }, { 0x08000001U, MEMORY_FLASH },
        { 0x08008000U, MEMORY_FLASH }, { 0x0800FFFEU, MEMORY_FLASH },
        { 0x0800FFFFU, MEMORY_FLASH }, { 0x08010000U, MEMORY_UNKNOWN },
        { 0x1FFFFFFFU, MEMORY_UNKNOWN },
        { 0x20000000U, MEMORY_SRAM }, { 0x20000001U, MEMORY_SRAM },
        { 0x20001000U, MEMORY_SRAM }, { 0x20001FFEU, MEMORY_SRAM },
        { 0x20001FFFU, MEMORY_SRAM }, { 0x20002000U, MEMORY_UNKNOWN },
        { 0x40020FFFU, MEMORY_UNKNOWN },
        { 0x40021000U, MEMORY_RCC }, { 0x40021001U, MEMORY_RCC },
        { 0x40021200U, MEMORY_RCC }, { 0x400213FEU, MEMORY_RCC },
        { 0x400213FFU, MEMORY_RCC }, { 0x40021400U, MEMORY_UNKNOWN },
        { 0x47FFFFFFU, MEMORY_UNKNOWN },
        { 0x48000000U, MEMORY_GPIOA }, { 0x48000001U, MEMORY_GPIOA },
        { 0x48000200U, MEMORY_GPIOA }, { 0x480003FEU, MEMORY_GPIOA },
        { 0x480003FFU, MEMORY_GPIOA }, { 0x48000400U, MEMORY_UNKNOWN },
        { UINT32_MAX, MEMORY_UNKNOWN }
    };
    size_t i;
    section = "M1 address boundaries";
    for (i = 0; i < sizeof cases / sizeof cases[0]; ++i) {
        MemoryRegion actual = memory_region(cases[i].address);
        if (actual != cases[i].expected) {
            fprintf(stderr, "  address 0x%08" PRIX32 ": expected %d, got %d\n",
                    cases[i].address, (int)cases[i].expected, (int)actual);
        }
        CHECK(actual == cases[i].expected);
    }
}

static void check_clock_values(void)
{
    CHECK(fake_hal.osc.OscillatorType == RCC_OSCILLATORTYPE_HSI);
    CHECK(fake_hal.osc.HSIState == RCC_HSI_ON);
    CHECK(fake_hal.osc.HSICalibrationValue == RCC_HSICALIBRATION_DEFAULT);
    CHECK(fake_hal.osc.PLL.PLLState == RCC_PLL_ON);
    CHECK(fake_hal.osc.PLL.PLLSource == RCC_PLLSOURCE_HSI);
    CHECK(fake_hal.osc.PLL.PREDIV == RCC_PREDIV_DIV1);
    CHECK(fake_hal.osc.PLL.PLLMUL == RCC_PLL_MUL12);
    CHECK(fake_hal.clock.ClockType ==
          (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1));
    CHECK(fake_hal.clock.SYSCLKSource == RCC_SYSCLKSOURCE_PLLCLK);
    CHECK(fake_hal.clock.AHBCLKDivider == RCC_SYSCLK_DIV1);
    CHECK(fake_hal.clock.APB1CLKDivider == RCC_HCLK_DIV1);
    CHECK(fake_hal.flash_latency == FLASH_LATENCY_1);
}

static void test_clock(void)
{
    static const HAL_StatusTypeDef errors[] = { HAL_ERROR, HAL_BUSY, HAL_TIMEOUT };
    static const FakeEvent both[] = { EVENT_OSC_CONFIG, EVENT_CLOCK_CONFIG };
    static const FakeEvent osc_only[] = { EVENT_OSC_CONFIG };
    size_t i;
    section = "C1 clock values and order";
    fake_hal_reset();
    CHECK(board_clock_init());
    expect_trace(both, sizeof both / sizeof both[0]);
    check_clock_values();
    for (i = 0; i < sizeof errors / sizeof errors[0]; ++i) {
        section = "C1 oscillator failure stops clock setup";
        fake_hal_reset();
        fake_hal.osc_result = errors[i];
        CHECK(!board_clock_init());
        expect_trace(osc_only, sizeof osc_only / sizeof osc_only[0]);
        section = "C1 clock switch failure";
        fake_hal_reset();
        fake_hal.clock_result = errors[i];
        CHECK(!board_clock_init());
        expect_trace(both, sizeof both / sizeof both[0]);
    }
}

static void check_led_values(void)
{
    CHECK(fake_hal.gpio_clock_enabled);
    CHECK(fake_hal.write_port == GPIOA);
    CHECK(fake_hal.write_pin == GPIO_PIN_5);
    CHECK(fake_hal.write_state == GPIO_PIN_RESET);
    CHECK(fake_hal.init_port == GPIOA);
    CHECK(fake_hal.gpio.Pin == GPIO_PIN_5);
    CHECK(fake_hal.gpio.Mode == GPIO_MODE_OUTPUT_PP);
    CHECK(fake_hal.gpio.Pull == GPIO_NOPULL);
    CHECK(fake_hal.gpio.Speed == GPIO_SPEED_FREQ_LOW);
    CHECK(fake_hal.gpio_initialized);
    CHECK(fake_hal.gpioa_output == (0xFFFFU & ~(uint32_t)GPIO_PIN_5));
}

static void test_led(void)
{
    static const FakeEvent init[] = {
        EVENT_GPIO_CLOCK_ENABLE, EVENT_GPIO_WRITE, EVENT_GPIO_INIT
    };
    static const FakeEvent after_toggle[] = {
        EVENT_GPIO_CLOCK_ENABLE, EVENT_GPIO_WRITE, EVENT_GPIO_INIT,
        EVENT_GPIO_TOGGLE
    };
    section = "L1 GPIO clock, initial output, configuration";
    fake_hal_reset();
    board_led_init();
    expect_trace(init, sizeof init / sizeof init[0]);
    check_led_values();
    section = "L2 only PA5 toggles";
    board_led_toggle();
    expect_trace(after_toggle, sizeof after_toggle / sizeof after_toggle[0]);
    CHECK(fake_hal.toggle_port == GPIOA);
    CHECK(fake_hal.toggle_pin == GPIO_PIN_5);
    CHECK(fake_hal.toggle_count == 1U);
    CHECK(fake_hal.gpioa_output == 0xFFFFU);
    CHECK(fake_hal.delay_count == 0U);
    board_led_toggle();
    CHECK(fake_hal.toggle_count == 2U);
    CHECK(fake_hal.gpioa_output == (0xFFFFU & ~(uint32_t)GPIO_PIN_5));
}

static void test_app_success(void)
{
    static const FakeEvent init[] = {
        EVENT_HAL_INIT, EVENT_OSC_CONFIG, EVENT_CLOCK_CONFIG,
        EVENT_GPIO_CLOCK_ENABLE, EVENT_GPIO_WRITE, EVENT_GPIO_INIT
    };
    static const FakeEvent after_two_steps[] = {
        EVENT_HAL_INIT, EVENT_OSC_CONFIG, EVENT_CLOCK_CONFIG,
        EVENT_GPIO_CLOCK_ENABLE, EVENT_GPIO_WRITE, EVENT_GPIO_INIT,
        EVENT_GPIO_TOGGLE, EVENT_DELAY, EVENT_GPIO_TOGGLE, EVENT_DELAY
    };
    section = "A1 application initialization";
    fake_hal_reset();
    CHECK(app_init());
    expect_trace(init, sizeof init / sizeof init[0]);
    check_clock_values();
    check_led_values();
    section = "A2 step toggles once then requests 250 ms";
    app_step();
    CHECK(fake_hal.toggle_count == 1U);
    CHECK(fake_hal.delay_count == 1U);
    CHECK(fake_hal.last_delay_ms == 250U);
    CHECK(fake_hal.gpioa_output == 0xFFFFU);
    app_step();
    expect_trace(after_two_steps, sizeof after_two_steps / sizeof after_two_steps[0]);
    CHECK(fake_hal.toggle_count == 2U);
    CHECK(fake_hal.delay_count == 2U);
    CHECK(fake_hal.last_delay_ms == 250U);
    CHECK(fake_hal.delay_total_ms == 500U);
    CHECK(fake_hal.gpioa_output == (0xFFFFU & ~(uint32_t)GPIO_PIN_5));
}

static void test_failed_reinitialization(void)
{
    static const HAL_StatusTypeDef errors[] = { HAL_ERROR, HAL_BUSY, HAL_TIMEOUT };
    static const FakeEvent until_hal[] = { EVENT_HAL_INIT };
    static const FakeEvent until_osc[] = { EVENT_HAL_INIT, EVENT_OSC_CONFIG };
    static const FakeEvent until_clock[] = {
        EVENT_HAL_INIT, EVENT_OSC_CONFIG, EVENT_CLOCK_CONFIG
    };
    const FakeEvent *sequences[] = { until_hal, until_osc, until_clock };
    const size_t lengths[] = { 1U, 2U, 3U };
    size_t failure_stage, i;
    for (failure_stage = 0; failure_stage < 3U; ++failure_stage) {
        for (i = 0; i < sizeof errors / sizeof errors[0]; ++i) {
            section = "A1 success state cleared on failed reinitialization";
            fake_hal_reset();
            CHECK(app_init()); /* Establish a previous successful state. */
            fake_hal_reset();
            if (failure_stage == 0U) {
                fake_hal.hal_result = errors[i];
            } else if (failure_stage == 1U) {
                fake_hal.osc_result = errors[i];
            } else {
                fake_hal.clock_result = errors[i];
            }
            CHECK(!app_init());
            expect_trace(sequences[failure_stage], lengths[failure_stage]);
            CHECK(!fake_hal.gpio_clock_enabled);
            CHECK(!fake_hal.gpio_initialized);
            app_step();
            app_step();
            expect_trace(sequences[failure_stage], lengths[failure_stage]);
            CHECK(fake_hal.toggle_count == 0U);
            CHECK(fake_hal.delay_count == 0U);
        }
    }
    section = "A1 recover after failed initialization";
    fake_hal_reset();
    CHECK(app_init());
    app_step();
    CHECK(fake_hal.toggle_count == 1U);
    CHECK(fake_hal.delay_count == 1U);
    CHECK(fake_hal.last_delay_ms == 250U);
    CHECK(fake_hal.protocol_errors == 0U);
}

int main(void)
{
    test_before_init(); /* Must precede the first app_init call in this process. */
    test_memory();
    test_clock();
    test_led();
    test_app_success();
    test_failed_reinitialization();
    printf("Public checks: %u, failures: %u\n", checks, failures);
    fflush(stdout);
    run_student_tests();
    puts("Student test function returned (review at least 2 assertions in PR).");
    if (failures != 0U) {
        puts("RESULT: FAIL - implement the TODOs; do not change fixed tests.");
        return 1;
    }
    puts("RESULT: PASS - Host API-contract tests only; no board execution.");
    return 0;
}
