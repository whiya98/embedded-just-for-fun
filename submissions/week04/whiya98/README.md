# Week 04 — MCU 요구값을 C 코드에 반영하기

이 폴더는 **STM32F030R8 / NUCLEO-F030R8용 요구사항을 구현하는 교육용 TODO Starter**입니다. STM32CubeF0의 공식 `GPIO_IOToggle` 예제를 참고하여 새로 작성했습니다. 공식 예제 원본이나 완성 Firmware를 그대로 복사한 자료가 아닙니다.

제출할 구현은 `src/*.c`의 Diff로 검토합니다. `include/mission.h`의 Public API와 고정 Test를 유지하고, `tests/test_student.c`에 최소 2개의 경계 또는 실패 Case를 추가하세요. Fork·Branch·PR 제출 경로는 이 주차 과제의 `SUBMISSION.md` 안내를 따르세요.

## 파일별 구현 요구사항

| 파일 | 함수 | 코드에 반영할 요구사항 |
|---|---|---|
| `src/memory_map.c` | `memory_region(uint32_t address)` | 아래 4개 영역의 시작·마지막 주소를 포함하여 분류. 그 외 `MEMORY_UNKNOWN`. 단일 주소 숫자만 판정하며 Pointer 역참조 금지. |
| `src/clock.c` | `board_clock_init()` | HSI 8 MHz / 2 × 12 = 48 MHz. HSI ON, 기본 Calibration, PLL ON, PREDIV DIV1. AHB/APB1 DIV1. `HAL_RCC_OscConfig` → `HAL_RCC_ClockConfig(..., FLASH_LATENCY_1)`. 어느 호출이든 `HAL_OK` 이외면 즉시 `false`. |
| `src/led.c` | `board_led_init()` | GPIOA Clock Enable → `HAL_GPIO_WritePin`으로 PA5 RESET → Output PP / No Pull / Low Speed 설정. 초기 LED OFF. |
| `src/led.c` | `board_led_toggle()` | `HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5)`으로 PA5만 한 번 Toggle. Delay는 이 함수의 책임이 아님. |
| `src/app.c` | `app_init()` | 매 호출 시작 시 성공 상태 해제. `HAL_Init` → `board_clock_init` → `board_led_init`. HAL/Clock 실패 시 이후 초기화를 중단하고 `false`. 모두 성공하면 `true`. |
| `src/app.c` | `app_step()` | 초기화 성공 상태에서만 LED Toggle 1회 → `HAL_Delay(250)`. 성공 상태를 Module 내부에서 관리. 실패 뒤 호출하면 아무 HAL 동작도 하지 않음. |
| `tests/test_student.c` | `run_student_tests()` | 최소 2개의 경계/실패 Case를 `assert`로 작성. 요구사항·입력·예상 결과·검증 이유를 주석으로 기록. |

`RCC_OscInitTypeDef`, `RCC_ClkInitTypeDef`, `GPIO_InitTypeDef`는 초기화한 뒤 필요한 필드를 채우세요. `HAL_ERROR`, `HAL_BUSY`, `HAL_TIMEOUT`은 모두 실패입니다. `app_init()` 실패 시 기존 Hardware 설정을 되돌리는 기능은 이번 범위에 포함하지 않으며, 실패 이후 새 GPIO 초기화나 `app_step()` 동작을 진행하지 않는 것이 계약입니다.

| 영역 | 첫 주소 | 마지막 주소 | 반환값 |
|---|---|---|---|
| Flash, 64 KiB | `0x08000000` | `0x0800FFFF` | `MEMORY_FLASH` |
| SRAM, 8 KiB | `0x20000000` | `0x20001FFF` | `MEMORY_SRAM` |
| GPIOA, 1 KiB Window | `0x48000000` | `0x480003FF` | `MEMORY_GPIOA` |
| RCC, 1 KiB Window | `0x40021000` | `0x400213FF` | `MEMORY_RCC` |

영역 판정은 모든 Offset의 Register 접근 가능 여부를 보장하지 않습니다. 예약 주소도 해당 Window 안에서는 같은 영역으로 분류합니다. Boot Alias `0x00000000`, 다른 GPIO Port 등 위 표에 없는 주소는 이 API에서 `MEMORY_UNKNOWN`입니다.

PA5는 Active HIGH LED입니다. `app_step()`을 연속 호출하면 명목상 250 ms마다 Toggle하므로 ON/OFF 한 주기는 약 500 ms입니다. Host Test의 `HAL_Delay`는 시간 값을 기록할 뿐 실제로 기다리지 않습니다. 실제 보드의 실행 시간이나 오차를 측정한 값이 아닙니다.

## Host에서 실행

C11 Compiler(`cc` 또는 `gcc`)와 GNU Make가 필요합니다. 이 폴더로 이동한 뒤 실행합니다.

```sh
make
make test
make stages
```

- `make`: TODO 상태에서도 Compile·Link가 성공해야 합니다.
- `make test`: TODO가 남은 최초 상태에서는 의도적으로 FAIL입니다. 구현 후 모든 Public Check와 학생 Test를 통과해야 합니다.
- `make stages`: `build/src/*.i`, `*.s`, `*.o`와 `build/mission-tests`를 생성합니다. Host Compiler의 Preprocess·Compile·Assemble·Link 산출물을 확인하세요. `make test`와 `make stages`는 모두 `.c → .i → .s → .o → Host Executable` 의존성을 사용합니다. 생성된 `.i`를 Compile하고, 생성된 `.s`를 Assemble하며, 중간 파일은 자동 삭제하지 않습니다.
- `build/`는 `.gitignore`에 포함되어 있습니다. Binary나 중간 산출물을 Commit하지 마세요.
- Compiler나 Flag를 바꾸면 `make clean` 후 다시 Build하세요.

Windows 10에서는 Git Bash만 설치해도 `make`와 `gcc`가 자동 제공되는 것은 아닙니다. 설치된 Compiler/Make를 확인하고, 필요한 경우 MSYS2/MinGW 개발 환경 등에서 실행하세요. 해당 도구가 준비된 Bash에서는 다음과 같이 Compiler와 확장자를 지정할 수 있습니다. Windows 실행은 이번 자료 작성 환경에서 검증하지 않았습니다.

```sh
gcc --version
make --version
make CC=gcc EXEEXT=.exe test
make CC=gcc EXEEXT=.exe stages
```

Test 출력을 PR에 남길 때는 성공 문구와 Exit Status를 함께 확인하세요. Bash에서 Pipe 없이 저장하는 예입니다.

```sh
make test > test.log 2>&1
result=$?
cat test.log
printf 'Exit status: %s\n' "$result"
```

## 고정 지원 코드와 검증 범위

`include/mission.h`, `host/*`, `tests/test_mission.c`, `Makefile`은 고정 지원 코드입니다. 결과를 맞추기 위해 수정하면 안 됩니다. 학생은 네 개의 `src/*.c`, `tests/test_student.c`, 제출 문서를 변경합니다.

`host/stm32f0xx_hal.h`는 이 과제에서 사용하는 STM32F030x8 HAL API 일부의 이름·형식을 제공하는 Test Adapter입니다. `GPIOA`는 Host의 일반 메모리 객체이며 실제 MCU 주소가 아닙니다. `fake_hal`은 함수 인자, 호출 순서, PA5 출력 상태, 실패 Return을 기록합니다. `fake_hal_reset()`은 Adapter 기록만 초기화하며 `app.c`의 내부 상태는 초기화하지 않습니다.

Host Test는 `.data/.bss` 초기화, 실제 Clock Lock, Interrupt, SysTick 주기, 전기적 LED 동작을 실행하지 않습니다. Host Executable의 형식도 OS에 따라 Mach-O / PE / ELF로 달라집니다. `make stages`는 MCU Firmware Build가 아닙니다. 실제 ARM Startup과 Linker는 고정된 공식 예제에서 별도로 추적하고 근거를 제출 문서에 적으세요. Target Build에서는 이 Host Header/Adapter를 사용하지 않습니다.

## 공식 근거

- [STM32F030R8 Datasheet, DS9773](https://www.st.com/resource/en/datasheet/stm32f030r8.pdf): Table 2, Figure 2, Figure 10, Table 17. 자료 기준 DS9773 Rev 5.
- [공식 GPIO_IOToggle 예제 main.c](https://github.com/STMicroelectronics/STM32CubeF0/blob/e220bfb12a162cfbf3bb65663e987e0d6550e9b4/Projects/STM32F030R8-Nucleo/Examples/GPIO/GPIO_IOToggle/Src/main.c): STM32CubeF0 v1.11.5 Commit `e220bfb12a162cfbf3bb65663e987e0d6550e9b4`.
- [공식 SW4STM32 Startup](https://github.com/STMicroelectronics/STM32CubeF0/blob/e220bfb12a162cfbf3bb65663e987e0d6550e9b4/Projects/STM32F030R8-Nucleo/Examples/GPIO/GPIO_IOToggle/SW4STM32/startup_stm32f030x8.s).
- [공식 HAL RCC Header](https://github.com/STMicroelectronics/stm32f0xx_hal_driver/blob/115eb1dc87e26ea29f4f2ca58003650381aabba2/Inc/stm32f0xx_hal_rcc.h): API의 구조체·상수 정의.

이번 제품 요구는 공식 예제와 차이가 있습니다. 공식 예제는 Delay 100 ms, Pull-up, High Speed를 사용하지만, 제출 코드는 **Delay 250 ms, No Pull, Low Speed, 초기 출력 OFF**를 구현해야 합니다. HSI 설정과 실패 Return 처리도 이번 API 계약에 맞춰 명시하세요.

## 제출자가 작성할 설명

- GitHub 아이디:
- 실제 PR URL: PR 생성 후 기입

### 모든 근본 원인과 변경 이유

| 요구사항·실패 상황 | 초안이 놓친 근본 원인 | 수정한 C File·함수 | 실제 검증 결과 |
| --- | --- | --- | --- |
| Memory 범위·경계 | | | |
| Clock 값·실패 전달 | | | |
| LED 설정·순서 | | | |
| 초기화·상태 수명·반복 동작 | | | |

### 본인이 추가한 Test

최소 2개 Case의 입력·예상값·실제값·추가한 이유를 적습니다.

### 실행 결과

- make test 결과와 Exit code:
- make stages 결과와 Exit code:
- 실제 Log: [test](logs/test.log), [stages](logs/stages.log)
- 미확인 사항:

### Review에서 설명할 내용

문제의 원인 → C 코드에서 바꾼 내용 → 근거 → 실행 검증 순서로 작성합니다.
