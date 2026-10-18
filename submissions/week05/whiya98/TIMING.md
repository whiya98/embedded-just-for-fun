# Timing 분석 기록

## 학습 목표와 공식 근거

`REFERENCES.md`의 자료 ID를 사용해 **학습 목표 → 자료의 위치·조건 → 찾은 사실 → 설계 → Test**를 연결합니다. 제품 사양 비교 대상은 TC387QP-160F300S, PG-LFBGA-292입니다. 실제 사용한 제품·Board Revision·Clock 설정이 다르면 차이를 기록하세요.

자료 ID: D0 Addendum, D1 Datasheet, U1 TC38x Appendix, U2 STM 설명, U3 Family Part 1, U4 Family Part 2, B1 Board, S1 Blinky 예제, S2 STM Header, S3 Port Header, S4 Scu Header. 상세 제목·Version·링크는 배포된 자료 안내를 기준으로 합니다.

| 학습 목표·확인 질문 | 자료 ID·Version·절/표/페이지 또는 File·Symbol | 적용 제품·조건 | 찾은 사실 | 설계·변경한 함수 | 검증 입력·예상 결과·실행 증거 |
| --- | --- | --- | --- | --- | --- |
| 목표 1 / GPIO Mode·Pin·극성 | D1 §2.2 Table 2-25 p160, D1 Fig.2-2 p13, B1 §3.15 p3-6, U3 §14.4.2 p14-8, §14.4.8 p14-21 | TC387QP-160F300S, PG-LFBGA-292, KIT_A2G_TC387_5V_TFT | ENABLE은 P00.0, LED D107은 P13.0에 연결된다. LED는 Active LOW이며 Port는 Input Pull-up 또는 Output Push-pull 등으로 설정할 수 있다. OMR을 통해 지정 Pin만 Set/Clear/Toggle할 수 있다. | `board_io_init()`, `board_collection_enabled()`, `board_led_toggle()`에서 LED는 HIGH로 준비 후 Push-pull 출력으로 설정하고, ENABLE은 Pull-up 입력으로 설정한다. LED Toggle 시 다른 Port Bit를 보존한다. | ENABLE LOW→true, HIGH→false. LED 초기값 HIGH(OFF). Toggle 후 대상 Bit만 변경되고 다른 Bit는 유지되는지 Test. |
| 목표 1 / STM Clock·Counter | U3 §10.5.1.3 p10-29, U4 §27.4.2 p27-9, U1 §25.2 Table 273 p25-1, S2 `IfxStm_getLower()`, S4 STM Clock 관련 Symbol | 과제 기본 `fSTM=100 MHz`; TIM0 하위 32-bit 사용 | STM Clock은 CPU Clock과 동일하다고 가정할 수 없고 `fSTM = fsource0 / STMDIV` 경로로 결정된다. STM 전체 Counter는 64-bit지만 TIM0를 읽으면 하위 32-bit 값을 사용한다. | `ticks_from_ms()`는 전달된 `stm_hz`를 사용하고, `period_due()`는 32-bit Counter wraparound를 고려한다. | 100 MHz/200 MHz 입력으로 Tick 계산 확인. `UINT32_MAX` 직전→0 이후의 Wraparound Case 검증. |
| 목표 2 / 주기 수집·만료 판정 | U4 §27.4.2 p27-9 및 과제 요구사항 | Single-core 순차 Polling, Sample 10 ms, LED Toggle 250 ms | Free-running Counter 값을 주기적으로 읽어 경과 Tick으로 만료 여부를 판정한다. 과제 정책상 늦게 호출되어도 한 호출에서 작업은 최대 1회 수행하고, 만료 시 기준 시각은 현재 `now`로 갱신한다. | `period_due()`, `collector_init()`, `collector_step()`에서 `now-last >= period` 방식으로 판단하고, 만료 시 `last=now`로 갱신한다. | 0→10 ms 수집 1회, 0→35 ms 호출 시에도 1회만 수행하고 기준은 35 ms. 이후 40 ms 미실행, 45 ms 실행. |
| 목표 3 / 대기·Polling·처리 시간 | S1 Blinky 예제의 대기 방식 및 과제 요구사항 | Host Simulation, 실제 Board Timing 아님 | Blocking Delay는 다른 작업의 확인을 늦출 수 있다. 이번 과제는 `collector_step()`에서 Delay/Sleep/대기 Loop를 사용하지 않고 호출 시점의 `now`로 만료 여부만 판단한다. | `collector_step()`을 Non-blocking Polling 구조로 작성한다. Sample과 LED 만료 판단을 독립적으로 수행한다. | 호출 간격이 늦어지면 만료 발견도 늦어질 수 있음을 시간표와 `make demo` 결과로 확인. |

- 공식 자료가 정한 사실과 과제에서 정한 정책을 구분한 설명:
  - 공식 자료는 STM Clock 경로, TIM0가 하위 32-bit Counter 값을 제공한다는 점, GPIO Pin/Mode/Register, Board의 LED Pin과 Active LOW 연결 등을 정한다.
  - 10 ms 수집 주기, 250 ms LED Toggle, 늦은 호출 시 최대 1회만 실행, 만료 시 `last = now`로 갱신하는 동작은 과제에서 정한 정책이다.
  - `uint64_t`를 이용한 중간 계산과 unsigned subtraction을 이용한 wraparound 처리는 C 정수 연산 특성과 과제 요구사항을 만족하기 위한 구현 판단이다.

- 아직 확인하지 못한 조건과 추가로 필요한 근거:
  - 실제 Board에서 사용되는 최종 STM Clock 값과 Startup/Clock 초기화 설정은 Host Test만으로 확인할 수 없다.
  - 실제 GPIO 전압과 Timing 정확도는 실제 Board와 계측 환경에서 별도 검증이 필요하다.
  - Host `platform_*` API는 실제 iLLD가 아니므로 실제 Board Adapter 적용 시 iLLD 호출과 Clock 설정을 다시 확인해야 한다.


## Tick 계산

100 MHz는 과제에서 제공하는 `fSTM` 조건입니다. **200 MHz는 고정값 사용을 검출하는 범용 산술의 Host 가상 입력**이며, TC387QP의 허용 STM 설정이나 실제 Board Clock을 뜻하지 않습니다. 두 입력을 실제 제품 사양으로 해석하지 마세요.

| STM Frequency | 10 ms Tick | 250 ms Tick | 계산 과정 |
| --- | ---: | ---: | --- |
| 100 MHz | 1,000,000 | 25,000,000 | `100,000,000 × 10 / 1000 = 1,000,000`, `100,000,000 × 250 / 1000 = 25,000,000` |
| 200 MHz | 2,000,000 | 50,000,000 | `200,000,000 × 10 / 1000 = 2,000,000`, `200,000,000 × 250 / 1000 = 50,000,000` |

- CPU Clock과 STM Clock을 같은 값으로 가정해도 되는가? 근거:
  - 가정하면 안 된다. D0에서 기준 Part인 TC387QP-160F300S의 최대 CPU Clock은 300 MHz이지만, U3 §10.5.1.3에 따르면 STM Clock은 `fsource0 / STMDIV` 경로로 결정된다.
  - 따라서 CPU 최대 주파수 300 MHz는 `fSTM` 값이 아니다.
  - 이번 과제에서는 전달받은 `stm_hz`를 사용하며, 기본 검증값 100 MHz는 과제가 제공한 조건이다.

- `hz × ms`를 32-bit로 먼저 계산했을 때의 실패 입력·실제 값:
  - 실패 입력 예: `hz = 100,000,000`, `ms = 250`
  - 기대 중간값: `25,000,000,000`
  - 이는 `UINT32_MAX = 4,294,967,295`를 초과한다.
  - 32-bit unsigned 연산으로 계산하면 중간 곱셈이 wraparound되어 `3,525,163,520`이 되고, 이를 1000으로 나누면 올바른 `25,000,000` Tick을 얻지 못한다.
  - 따라서 중간 곱셈은 `uint64_t` 범위에서 수행해야 한다.

- 정수가 아닌 Tick 또는 표현 범위를 벗어난 값의 처리:
  - `hz × ms`가 1000으로 정확히 나누어지지 않으면 정수 Tick이 아니므로 실패한다.
  - 계산 결과가 1 Tick 미만이거나 `INT32_MAX`를 초과하면 실패한다.
  - 실패 시 `false`를 반환하며 `out`이 유효한 경우 기존 `*out` 값은 보존한다.


## 주기 판단

아래는 1 Tick = 1 ms인 별도 분석 입력입니다. 시작 기준 시각은 0, 주기는 10 Tick입니다.

| now | 실행 여부 | 호출 이후 last | 판단 근거 |
| ---: | --- | ---: | --- |
| 9 | 실행 안 함 | 0 | 경과 9 Tick < 10 Tick |
| 10 | 실행 | 10 | 경과 10 Tick >= period, 만료 후 `last = now` |
| 35 | 실행 | 35 | 이전 기준 10에서 25 Tick 경과. 늦은 호출이어도 한 번만 실행하고 기준을 35로 갱신 |
| 36 | 실행 안 함 | 35 | 새 기준에서 1 Tick 경과 |
| 45 | 실행 | 45 | 새 기준 35에서 정확히 10 Tick 경과 |

- Counter Wrap 직전·직후에 비교할 값:
  - 예: `last = 0xFFFFFFFA`, `period = 10`, `now = 0x00000004`
  - unsigned 32-bit subtraction으로 `now - last = 10` Tick이 된다.
  - 따라서 Wraparound가 발생했어도 주기가 만료된 것으로 판단할 수 있다.
  - 단, 마지막 기준 이후 실제 경과 시간이 `2^32` Tick 미만이라는 과제 조건 안에서만 유효하다.

- 관측을 Counter 한 바퀴보다 오래 중단하면 알 수 없는 정보:
  - TIM0의 하위 32-bit 값만 비교하므로 Counter가 한 바퀴 이상 돌았는지, 여러 바퀴 돌았는지를 두 32-bit 값만으로 복구할 수 없다.
  - 따라서 이번 과제에서는 실제 경과 시간이 `2^32` Tick 미만이라고 제한한다.

- Disabled 중 기준 시각을 갱신하는 이유:
  - ENABLE이 HIGH여서 수집하지 않더라도 Sample 주기의 만료 판정과 기준 시각 갱신은 계속해야 한다.
  - 그렇지 않으면 다시 ENABLE이 LOW가 되었을 때 비활성 기간에 놓친 Sample을 몰아서 수행하는 Catch-up 동작이 발생할 수 있다.
  - 과제 요구사항은 실제로 읽은 Sample만 `sample_count`에 포함하고, 과거 시점을 복원하지 않는 것이다.

- 250 ms Toggle 간격과 LED ON/OFF 전체 주기의 차이:
  - LED 상태는 250 ms마다 한 번 Toggle된다.
  - OFF→ON 또는 ON→OFF는 250 ms지만, OFF→ON→OFF의 한 전체 주기는 500 ms이다.

- Polling 호출이 늦어질 때 보장되는 것과 보장되지 않는 것:
  - 주기가 지난 뒤 `collector_step()`이 호출되면 만료 사실을 확인하고 해당 작업을 한 번 수행할 수 있다.
  - 하지만 `collector_step()` 호출 자체가 늦어지면 실제 Sample 시각도 늦어진다.
  - 이번 설계는 정확히 10 ms마다 실행되는 실시간 스케줄러를 보장하는 것이 아니라, 호출 시점에서 만료 여부를 판단하는 Polling 방식이다.
  - 한 번의 호출에서 누락된 횟수만큼 Catch-up하지 않는다.


## 실제 Demo 결과

`make demo` 실행 후 `make -s demo > timing.csv`로 CSV를 저장하고, 위 분석과 결과를 비교합니다.

`make demo` 실행 결과:

- 10 ms에서 첫 Sample이 수행되었다.
- 35 ms의 늦은 호출에서는 누락된 Sample을 Catch-up하지 않고 1회만 수행하였다.
- 이후 기준 시각이 35 ms로 갱신되어 36 ms에는 수집하지 않고 45 ms에 다시 수집하였다.
- ENABLE이 비활성인 250 ms와 260 ms에는 Sample 수가 증가하지 않았다.
- 수집 비활성 상태에서도 250 ms에 LED Toggle은 정상 수행되었다.
- 270 ms에서 ENABLE을 다시 활성화한 뒤 Sample은 1회만 수행되었다.
- 모든 시점에서 `delay_calls = 0`으로 Blocking Delay가 사용되지 않았음을 확인하였다.

- CSV: `logs/demo.csv`
- 불일치: 없음
- 본 결과는 실제 TC387QP Board의 시간 측정 결과가 아니라 Host에서 입력한 가상 시각을 이용한 Simulation 결과이다.