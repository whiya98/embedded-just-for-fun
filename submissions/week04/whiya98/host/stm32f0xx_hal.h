#ifndef MISSION_HOST_STM32F0XX_HAL_H
#define MISSION_HOST_STM32F0XX_HAL_H

/* FIXED TEST SUPPORT: original teaching adapter, not the ST HAL distribution.
   Only the STM32F030x8 API subset used by this assignment is provided.
   GPIOA points to ordinary Host memory. It is NEVER a real MMIO address.
   Do not use this header when compiling firmware for a board. */
#include <stdint.h>

typedef enum { HAL_OK = 0, HAL_ERROR = 1, HAL_BUSY = 2, HAL_TIMEOUT = 3 }
    HAL_StatusTypeDef;
typedef enum { GPIO_PIN_RESET = 0, GPIO_PIN_SET = 1 } GPIO_PinState;
typedef struct { uint32_t unused; } GPIO_TypeDef;
extern GPIO_TypeDef mission_host_gpioa;
#define GPIOA (&mission_host_gpioa)
#define GPIO_PIN_5 ((uint16_t)0x0020U)
#define GPIO_PIN_6 ((uint16_t)0x0040U)
#define GPIO_MODE_OUTPUT_PP 0x00000001U
#define GPIO_NOPULL 0x00000000U
#define GPIO_PULLUP 0x00000001U
#define GPIO_SPEED_FREQ_LOW 0x00000000U
#define GPIO_SPEED_FREQ_HIGH 0x00000003U

typedef struct {
    uint32_t Pin, Mode, Pull, Speed, Alternate;
} GPIO_InitTypeDef;
typedef struct {
    uint32_t PLLState, PLLSource, PLLMUL, PREDIV;
} RCC_PLLInitTypeDef;
typedef struct {
    uint32_t OscillatorType, HSEState, LSEState, HSIState, HSICalibrationValue;
    uint32_t HSI14State, HSI14CalibrationValue, LSIState;
    RCC_PLLInitTypeDef PLL;
} RCC_OscInitTypeDef;
typedef struct {
    uint32_t ClockType, SYSCLKSource, AHBCLKDivider, APB1CLKDivider;
} RCC_ClkInitTypeDef;

#define RCC_OSCILLATORTYPE_NONE 0x00000000U
#define RCC_OSCILLATORTYPE_HSI 0x00000002U
#define RCC_HSI_ON 0x00000001U
#define RCC_HSICALIBRATION_DEFAULT 0x00000010U
#define RCC_PLL_ON 0x00000002U
#define RCC_PLLSOURCE_HSI 0x00000000U
#define RCC_PREDIV_DIV1 0x00000000U
#define RCC_PLL_MUL12 0x00280000U
#define RCC_CLOCKTYPE_SYSCLK 0x00000001U
#define RCC_CLOCKTYPE_HCLK 0x00000002U
#define RCC_CLOCKTYPE_PCLK1 0x00000004U
#define RCC_SYSCLKSOURCE_PLLCLK 0x00000002U
#define RCC_SYSCLK_DIV1 0x00000000U
#define RCC_HCLK_DIV1 0x00000000U
#define FLASH_LATENCY_1 0x00000001U

HAL_StatusTypeDef HAL_Init(void);
HAL_StatusTypeDef HAL_RCC_OscConfig(RCC_OscInitTypeDef *config);
HAL_StatusTypeDef HAL_RCC_ClockConfig(RCC_ClkInitTypeDef *config,
                                    uint32_t latency);
void mission_host_gpioa_clock_enable(void);
#define __HAL_RCC_GPIOA_CLK_ENABLE() mission_host_gpioa_clock_enable()
void HAL_GPIO_WritePin(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState state);
void HAL_GPIO_Init(GPIO_TypeDef *port, GPIO_InitTypeDef *config);
void HAL_GPIO_TogglePin(GPIO_TypeDef *port, uint16_t pin);
void HAL_Delay(uint32_t milliseconds);

#endif
