# 4주차 Datasheet · Build · Startup 추적

## 1. Datasheet 근거와 MCU 관계

| 구현한 값 | Datasheet 페이지·표·그림 또는 BSP 근거 | C File·함수와 사용 의미 |
| --- | --- | --- |
| Flash 영역 | DS9773 Rev 5, Table 2: STM32F030R8 Flash = 64 KiB. Figure 10: Flash memory 시작 주소 = `0x08000000` | `src/memory_map.c`, `memory_region()`. `0x08000000 ~ 0x0800FFFF`를 `MEMORY_FLASH`로 분류한다. 64 KiB = `0x10000` Byte이므로 끝 주소는 `0x08000000 + 0x10000 - 1 = 0x0800FFFF`이다. |
| SRAM 영역 | DS9773 Rev 5, Table 2: STM32F030R8 SRAM = 8 KiB. Figure 10: SRAM 시작 주소 = `0x20000000` | `src/memory_map.c`, `memory_region()`. `0x20000000 ~ 0x20001FFF`를 `MEMORY_SRAM`으로 분류한다. 8 KiB = `0x2000` Byte이므로 끝 주소는 `0x20001FFF`이다. |
| GPIOA 영역 / LED2 Port·Pin | DS9773 Rev 5, Table 17: GPIOA = `0x48000000 ~ 0x480003FF`, 1 KiB. STM32F0xx Nucleo BSP `stm32f0xx_nucleo.h`: `LED2_PIN = GPIO_PIN_5`, `LED2_GPIO_PORT = GPIOA`, `LED2_GPIO_CLK_ENABLE() = __HAL_RCC_GPIOA_CLK_ENABLE()` | `src/memory_map.c`, `memory_region()`에서는 GPIOA 주소 영역을 `MEMORY_GPIOA`로 분류한다. `src/led.c`, `board_led_init()` 및 `board_led_toggle()`에서는 LED2를 GPIOA Pin 5로 사용하고 GPIOA Clock Enable 후 초기화·제어한다. |
| RCC 영역 | DS9773 Rev 5, Table 17: RCC = `0x40021000 ~ 0x400213FF`, 1 KiB | `src/memory_map.c`, `memory_region()`에서 해당 범위를 `MEMORY_RCC`로 분류한다. RCC는 Clock source와 Bus/Peripheral Clock 설정 및 GPIOA Clock Enable에 사용되는 제어 블록이다. |
| HSI·PLL·SYSCLK·Bus Clock | DS9773 Rev 5 §3.6: Reset 후 기본 CPU Clock은 내부 HSI 8 MHz. Figure 2: STM32F030x8 Clock tree에서 HSI를 `/2`하여 PLL 입력으로 사용할 수 있고 PLL 배수는 x2~x16, PLLCLK를 SYSCLK로 선택할 수 있다. AHB와 APB에는 Prescaler가 존재하며 최대 48 MHz까지 동작한다. | `src/clock.c`, `board_clock_init()`. HSI 8 MHz → HSI/2 = 4 MHz → PLL ×12 = 48 MHz → PLLCLK를 SYSCLK로 선택한다. AHB와 APB1은 /1로 설정하여 HCLK = 48 MHz, PCLK = 48 MHz가 되도록 한다. |

STM32F030R8은 STM32F030 family 전체 최대 사양과 구분해서 사용했다. Family 전체는 최대 256 KiB Flash와 최대 32 KiB SRAM을 지원하지만, STM32F030R8은 Table 2 기준 Flash 64 KiB, SRAM 8 KiB를 갖는다. 

GPIOA와 RCC 주소 범위는 Table 17에서 직접 확인했으며, GPIOA는 `0x48000000 ~ 0x480003FF`, RCC는 `0x40021000 ~ 0x400213FF`이다. 

NUCLEO 보드 BSP에서는 LED2가 `GPIOA`의 `GPIO_PIN_5`로 정의되어 있고, GPIOA Clock Enable 매크로도 함께 정의되어 있다. 


CPU·Flash·SRAM·RCC·GPIOA 관계도를 아래에 작성합니다. 주소 접근과 Clock 공급·Enable 화살표의 의미를 구분하세요.

```text
                        +------------------+
                         | Cortex-M0 CPU    |
                         +---------+--------+
                                   |
                                   | 주소 기반 Read/Write
                                   | (Bus Matrix / AHB)
                                   v
                 +-----------------+------------------+
                 |                                    |
                 v                                    v
          +-------------+                      +-------------+
          | Flash       |                      | SRAM        |
          | Program/Data|                      | Read/Write  |
          +-------------+                      +-------------+

                                   |
                                   | AHB Register Access
                                   v
                         +------------------+
                         | RCC              |
                         | Reset & Clock    |
                         | Control          |
                         +---------+--------+
                                   |
                                   | GPIOA Clock Enable
                                   | / Clock 공급
                                   v
                         +------------------+
                         | GPIOA            |
                         | PA5 -> LED2      |
                         +------------------+
                                   ^
                                   |
                                   | CPU가 AHB를 통해
                                   | GPIO Register 접근
                                   |
                             Cortex-M0 CPU
```

Clock 계산식과 공개 예제에서 선택한 경로:

HSI = 8 MHz
PLL input = HSI / 2
          = 8 MHz / 2
          = 4 MHz

PLLCLK = 4 MHz × 12
       = 48 MHz

SYSCLK = PLLCLK
       = 48 MHz

AHB Prescaler = /1
HCLK = 48 MHz

APB1 Prescaler = /1
PCLK = 48 MHz

STM32F030R8은 최대 48 MHz의 CPU 동작 주파수를 지원한다. 다만 최대 동작 주파수가 48 MHz라는 것은 Reset 직후부터 CPU가 48 MHz로 동작한다는 의미는 아니다.

Datasheet의 Clock 관련 설명에 따르면 Reset 직후에는 내부 8 MHz RC oscillator인 HSI가 기본 System Clock source로 사용된다. 따라서 별도의 Clock 설정을 수행하지 않은 초기 상태에서는 HSI 8 MHz를 기준으로 동작한다.


## 2. 공개 예제와 제품 요구의 차이

| 항목 | 공개 예제의 설정·근거 | 이번 제품의 요구·변경한 C 위치 |
| --- | --- | --- |
| Clock | | |
| LED 초기 상태·Mode·Pull·Speed | | |
| Toggle 간격 | | |
| 실패 처리 | | |


Datasheet의 Clock Tree에서는 HSI 8 MHz가 /2 경로를 통해 PLL 입력으로 사용될 수 있으며, PLL multiplier를 이용해 높은 주파수를 생성한 뒤 PLLCLK를 SYSCLK source로 선택할 수 있음을 확인하였다. 또한 AHB와 APB prescaler를 통해 각각 HCLK과 PCLK1을 결정할 수 있다.

따라서 본 구현에서는 HSI를 기반으로 PLL을 사용하여 48 MHz Clock을 생성하고, 이를 SYSCLK으로 선택한 뒤 AHB와 APB1을 모두 /1로 설정하였다.

### 최대 주파수와 Reset 직후 Clock의 차이

MCU의 최대 주파수는 해당 MCU가 정상 동작을 보장하는 최대 Clock 주파수를 의미한다. 반면 Reset 직후 Clock은 MCU가 Reset 해제 직후 기본적으로 선택하는 Clock source와 주파수를 의미한다.

STM32F030R8의 최대 CPU 주파수는 48 MHz이지만, Reset 직후에는 HSI 8 MHz가 기본 Clock으로 사용된다. 따라서 48 MHz로 동작시키기 위해서는 PLL과 System Clock source를 별도로 설정해야 한다.


## 3. 공개 STM32 Project의 Build

| 확인할 내용 | 실제 File·설정·근거 |
| --- | --- |
| Target / Compiler 계열 / Version 명시 여부 | |
| Source 입력 / Include / Define | |
| Startup Object / Linker script / Library | |
| 실행 Image와 후처리 산출물 | |

Header를 포함하는 것과 `.c`를 Build 입력에 넣는 것의 차이:

## 4. 공개 예제의 Reset부터 main까지

| 실제 순서 | File·함수·label | 준비하는 상태와 다음 단계의 관계 |
| --- | --- | --- |
| | | |

`SystemInit`과 `SystemClock_Config`의 호출 위치·역할 차이:

## 5. 내가 실행한 Host Build

| 단계 | 실제 입력 File | 실제 출력 File | Log의 명령·옵션과 역할 |
| --- | --- | --- | --- |
| Preprocess | | | |
| Compile | | | |
| Assemble | | | |
| Link | | | |

Host에서 검증한 내용 / ARM Source로만 확인한 내용 / Board에서 아직 확인하지 않은 내용:

## 6. Module과 초기화

네 `.c`의 책임과 호출 방향, 공개 API와 내부 상태를 설명합니다.

- memory_map.c:
- clock.c:
- led.c:
- app.c:

초기화 성공·실패·재초기화 실패 시 호출 순서와 `app_step` 동작:
