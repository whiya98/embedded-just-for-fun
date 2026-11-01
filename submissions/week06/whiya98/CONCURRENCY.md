# 06회차 동시성·지연 근거 기록

## 1. 문서에서 확인한 근거

이번 과제의 대상은 Infineon AURIX TC387QP(TC38x 계열)이며, CPU0의 Main과 하나의 Timer ISR이 공유 Mailbox에 접근하는 환경을 가정한다.

TC387QP Datasheet와 TC3xx User Manual을 통해 하드웨어 동작을 확인하고, Infineon STM Interrupt 공식 예제 및 iLLD API를 참고하여 구현 방향을 결정하였다.

| 확인할 사실 | 공식 문서·Version·위치 | 문서의 조건 | 구현 판단 | 확인 Test |
|---|---|---|---|---|
| TC387QP의 STM / Interrupt 경로 | TC38x Data Sheet Addendum v1.9, Table 1 p3 / TC3xx User Manual Part 1 V2.0.0, §16.4.1 | TC387QP는 TC38x 계열. STM Compare 요청은 Interrupt Router를 통해 지정된 Service Provider로 전달 | Timer 요청을 CPU0 ISR에서 처리하도록 모델링. CPU 최대 주파수를 STM Clock으로 가정하지 않음 | Timer ISR 호출 및 요청 기록 검증 |
| STM Compare 요청 처리와 다음 Compare 갱신 | TC3xx User Manual Part 2 V2.0.0, §27.3.2, §27.4.4 / STM_Interrupt.c, `isrSTM()` / IfxStm.h, `IfxStm_increaseCompare()` | STM은 Free-running Counter이며 Compare Match로 요청을 발생시킴. 공식 예제는 ISR에서 다음 Compare를 갱신 | `platform_timer_service()`를 ISR 첫 동작으로 실행. 이후 Tick을 읽고 Mailbox에 기록 | `timer_isr()` 호출 순서, 호출 횟수, Trace 검증 |
| CPU0 IRQ 차단과 이전 상태 복원 | TC3xx User Manual Part 1, Interrupt Router / AP32381, CPU Interrupt and Trap handling / IfxCpu.h, `IfxCpu_disableInterrupts()`, `IfxCpu_restoreInterrupts()` | CPU Interrupt 허용 상태를 저장하고 차단한 뒤 이전 상태로 복원할 수 있음 | `platform_irq_save_disable()` 반환값을 저장해 `platform_irq_restore(previous)`에 전달 | IRQ Disabled 상태 유지 및 Snapshot Race Test |
| `volatile` 접근과 복합 Read/Clear의 차이 | GCC Volatiles / ISO C11 N1570 §6.7.3, §6.5.2.4 | `volatile`은 여러 공유 변수의 읽기와 초기화 전체에 대한 원자성을 보장하지 않음 | Mailbox의 세 필드 읽기와 초기화를 하나의 Critical section 안에서 수행 | `platform_interleave()`를 이용한 Race 재현 |

### 공식 예제와 과제 요구사항의 구분

Infineon의 `STM_Interrupt_1_KIT_TC387_TFT` 예제는 STM0와 CPU0를 사용하며, Priority 40과 500ms 주기를 설정한다.

공식 예제의 `IFX_INTERRUPT()`는 ISR과 Vector Table 및 Priority를 연결하고, `IfxStm_increaseCompare()`는 다음 Compare 값을 갱신한다. `typeOfService`에는 CPU0가 지정되어 있다.

이번 과제에서 사용하는 10ms 주기와 Mailbox 용량 4는 하드웨어 고정 사양이 아닌 별도의 제품 요구사항이다.

또한 STM Compare Flag, Interrupt Router의 Service Request, Software Mailbox의 pending은 서로 다른 상태이다. Mailbox의 pending은 실제 실행된 ISR에서 수락한 요청 개수를 기록하는 소프트웨어 변수이다.

## 2. Polling과 Interrupt 처리 흐름

### 2.1 Polling 방식

```text
Main 시작
    |
    v
Timer 상태 또는 Tick 확인
    |
    +-- 10ms 미경과 --> 다시 확인
    |
    +-- 10ms 경과
           |
           v
       Sensor Read
           |
           v
         Publish
           |
           v
       Main 반복
```

Polling은 Main이 직접 Timer 상태를 확인하는 방식이다.

구현이 비교적 단순하고 Interrupt Context와의 공유 데이터 문제를 줄일 수 있다는 장점이 있다.

그러나 Main에서 오래 걸리는 작업을 수행하면 다음 Timer 상태를 확인하는 시점이 지연된다.

특히 여러 주기가 경과했을 때 개별 요청을 모두 복원할 수 있는지는 Timer Flag와 소프트웨어 구현 방식에 따라 달라진다.

따라서 주기 정확성 요구가 높지 않고 Main의 실행 시간이 충분히 짧은 경우에 적합하다.

### 2.2 Interrupt 방식

```text
STM Timer Compare Match
           |
           v
       IRQ 발생
           |
           v
     CPU0 ISR 실행
           |
           v
 platform_timer_service()
           |
           v
   platform_timer_now()
           |
           v
     event_post_isr()
           |
           v
     Mailbox에 기록
           |
           v
       ISR 종료
           |
           v
     Main 실행 계속
           |
           v
     collector_step()
           |
           v
   Mailbox Snapshot
           |
           v
       Sensor Read
           |
           v
         Publish
```

Interrupt 방식은 Timer 이벤트가 발생하면 CPU에 Interrupt 처리를 요청하는 방식이다.

Main이 다른 작업을 수행하더라도 IRQ가 허용되어 있다면 ISR이 실행되어 요청을 기록할 수 있다.

이번 과제에서는 Sensor read와 Publish 시간이 Timer 요청 주기보다 길 수 있으므로 Interrupt 방식을 선택하였다.

다만 Interrupt가 차단되어 있거나 ISR 처리가 오래 걸리면 응답 지연이 발생한다. Interrupt 방식 자체가 모든 주기의 처리를 보장하는 것은 아니다.

## 3. ISR과 Main 업무 분리

| 구분 | Timer ISR | Main |
|---|---|---|
| 실행 조건 | Timer Interrupt 수락 | Main의 `collector_step()` 호출 |
| Timer 서비스 | 수행 | 수행하지 않음 |
| 요청 시각 기록 | 수행 | Snapshot으로 가져옴 |
| Mailbox 접근 | 요청 누적 | 요청 가져오기 및 초기화 |
| Sensor read | 수행하지 않음 | 유효 Batch당 1회 |
| Publish | 수행하지 않음 | 유효 Batch당 1회 |
| Delay | 수행하지 않음 | 수행하지 않음 |
| IRQ 상태 | ISR Context | I/O 시 Enabled |

### 개선 내용

**timer_isr.c**

기존 ISR에서 수행하던 Sensor read, Publish, Delay를 제거하였다.

`platform_timer_service()`를 가장 먼저 실행하고 현재 Tick을 읽어 `event_post_isr()`에 전달하도록 수정하였다.

**collector.c**

`event_take()`로 가져온 Batch를 이용하여 Report를 생성하도록 수정하였다.

- `request_tick = batch.last_tick`
- `requests = batch.pending`
- `coalesced = batch.pending - 1`
- `dropped = batch.dropped`

센서 읽기 및 Publish 과정에서 별도로 IRQ를 차단하지 않도록 변경하였다.

**검증 결과**

Host Test에서 ISR 실행 순서와 I/O Context를 검증하였다.

Demo의 모든 시나리오에서 `io_in_isr=0`, `io_irq_disabled=0`이 확인되었다.

## 4. 경쟁 순서 재현

### 4.1 기존 코드의 Race condition

기존 `event_take()`는 Mailbox를 읽은 후 IRQ를 활성화하고, 그다음 공유 데이터를 초기화하였다.

따라서 읽기와 초기화 사이에 ISR이 실행되면 새 요청이 삭제될 수 있다.

| 순서 | 실행 주체 | IRQ 상태 | Mailbox (pending, dropped, last_tick) | 지역 Snapshot | 근거 |
|---|---|---|---|---|---|
| 1 | ISR | Enabled | (1, 0, 10) | 없음 | tick 10 요청 |
| 2 | Main | Disabled | (1, 0, 10) | (1, 0, 10) | 공유 데이터 읽기 |
| 3 | Main | Enabled | (1, 0, 10) | (1, 0, 10) | 원본 코드의 조기 IRQ 복원 |
| 4 | ISR | Enabled | (2, 0, 20) | (1, 0, 10) | tick 20 요청 추가 |
| 5 | Main | Enabled | (0, 0, 0) | (1, 0, 10) | 공유 데이터 초기화 |
| 6 | Main | Enabled | (0, 0, 0) | (1, 0, 10) | tick 20 요청 유실 |

위 표는 요청 누적 기능이 정상이라고 가정하여 읽기·초기화 경쟁만 분리한 실행 순서이다.

tick 20 요청은 Snapshot에 포함되지 않았으며, Mailbox에서도 초기화되어 다음 Main 처리에서 확인할 수 없게 된다.

### 4.2 개선 코드의 Critical section

개선 후에는 IRQ를 차단한 상태에서 세 필드를 Snapshot으로 복사하고, `platform_interleave()`를 한 번 실행한 뒤 Mailbox를 초기화한다.

| 순서 | 실행 주체 | IRQ 상태 | Mailbox (pending, dropped, last_tick) | 지역 Snapshot | 근거 |
|---|---|---|---|---|---|
| 1 | ISR | Enabled | (1, 0, 10) | 없음 | tick 10 요청 기록 |
| 2 | Main | Disabled | (1, 0, 10) | (1, 0, 10) | IRQ 차단 및 Snapshot 확보 |
| 3 | Hook | Disabled | (1, 0, 10) | (1, 0, 10) | tick 20 IRQ Pending 보류 |
| 4 | Main | Disabled | (0, 0, 0) | (1, 0, 10) | Mailbox 초기화 |
| 5 | Main / ISR | Enabled | (1, 0, 20) | (1, 0, 10) | 이전 IRQ 복원 후 보류 ISR 실행 |
| 6 | Main | Enabled | (1, 0, 20) | (1, 0, 10) | 기존 Snapshot 반환 |

이렇게 하면 이번 Snapshot은 tick 10 요청만 포함하고, tick 20 요청은 다음 Batch에 남는다.

Demo의 `snapshot_race`에서 `request_tick=10`, `requests=1`, `next_pending=1`을 확인하였다. 이어지는 `next_request`에서는 `request_tick=20`, `requests=1`이 확인되었다.

### 4.3 IRQ 이전 상태 보존

```c
bool previous_irq = platform_irq_save_disable();

/* Snapshot 읽기와 Mailbox 초기화 */

platform_irq_restore(previous_irq);
```

기존 코드의 `platform_irq_restore(true)`는 진입 전 IRQ 상태와 관계없이 IRQ를 활성화할 수 있었다.

개선 코드에서는 이전 IRQ 상태를 저장하고 이를 복원한다.

- 이전 Enabled → 처리 후 Enabled
- 이전 Disabled → 처리 후 Disabled

`event_take(NULL)`에서는 IRQ 상태를 변경하지 않고 즉시 false를 반환한다.

단, 이번 보호 방식은 CPU0의 Main과 단일 Timer ISR이 공유 데이터에 접근한다는 조건을 전제로 한다. 다른 Core, DMA 또는 Trap까지 차단하는 범용 동기화 수단은 아니다.

### 4.4 volatile과 원자성

`volatile`은 공유 변수 접근을 컴파일러가 임의로 생략하거나 최적화하는 것을 제한하지만, 여러 필드에 대한 복합 작업의 원자성까지 보장하지 않는다.

예를 들어 다음 동작은 하나의 원자적 작업이 아니다.

```c
snapshot.pending = g_events.pending;
snapshot.dropped = g_events.dropped;
snapshot.last_tick = g_events.last_tick;

g_events.pending = 0u;
g_events.dropped = 0u;
g_events.last_tick = 0u;
```

따라서 ISR이 중간에 실행되면 서로 다른 시점의 값이 섞이거나 새로운 요청이 삭제될 수 있다.

이번 구현에서는 Critical section으로 공유 데이터를 읽고 초기화하는 전체 구간을 보호하였다.

## 5. 지연 분석

### 5.1 분석 조건

- Timer 요청 주기: 10ms
- Sensor read: 2ms
- Publish: 15ms
- Sensor read + Publish: 17ms
- 기존 ISR의 추가 Delay: 10ms
- 개선된 Snapshot Critical section 시간: B

위 시간은 과제의 분석용 가정이며 실제 MCU 측정값이 아니다.

### 5.2 기존 ISR에서 I/O를 수행하는 경우

Sensor read와 Publish만 수행해도 17ms가 필요하다.

이는 Timer 요청 주기인 10ms를 7ms 초과한다.

기존 ISR의 Delay 10ms까지 포함하면 총 27ms이며, Timer 서비스와 기타 코드 실행 시간은 별도로 추가된다.

ISR 실행 중 동일 CPU에서 다른 Interrupt를 허용하지 않는 경우, 다음 Timer 요청의 서비스가 지연될 수 있다.

또한 ISR 내부의 긴 실행 시간은 Main 복귀와 다른 Interrupt의 응답에도 영향을 준다.

다만 실제 지연량은 Interrupt 우선순위, 중첩 허용 여부 및 하드웨어 조건에 따라 달라진다.

### 5.3 Main에서 IRQ Disabled로 I/O를 수행하는 경우

Main이 Sensor read 2ms와 Publish 15ms를 IRQ Disabled 상태에서 수행하면 17ms 동안 ISR 실행이 차단될 수 있다.

이는 10ms 요청 주기보다 긴 차단 구간이다.

따라서 Timer 요청이 발생하더라도 즉시 ISR을 실행할 수 없고, 여러 요청이 발생하면 일부가 병합되거나 누락될 수 있다.

### 5.4 Main에서 IRQ Enabled로 I/O를 수행하는 경우

개선 코드에서는 Mailbox Snapshot을 가져온 후 IRQ를 복원하고 Sensor read와 Publish를 수행한다.

따라서 Main에서 17ms 동안 I/O를 수행하더라도 IRQ가 허용되어 있으면 Timer ISR이 실행되어 새로운 요청을 Mailbox에 기록할 수 있다.

Main 처리 시간이 10ms보다 길어도, 새로운 요청이 전부 즉시 보고되는 것은 아니다. 요청은 Mailbox에 누적되며 Main이 다음에 처리할 때 병합된다.

### 5.5 Critical section 시간 B

개선된 `event_take()`에서는 세 필드 Snapshot 복사와 초기화, Hook 및 관련 제어 동작을 하나의 Critical section으로 보호한다.

이 구간의 실행 시간을 B라고 정의한다.

해당 구간에서 IRQ가 발생하면, Critical section에 의해 추가되는 IRQ 차단 대기 시간은 최대 B이다.

단, 이는 해당 Critical section 하나로 인한 추가 차단 시간의 상한이며 전체 Interrupt Latency가 B라는 의미는 아니다.

전체 Latency에는 다음 요소들이 영향을 줄 수 있다.

- Timer Compare 발생과 IRQ 전달 과정
- CPU Interrupt 수락 및 Context 처리
- 다른 Interrupt 또는 우선순위 영향
- IRQ 차단 구간의 잔여 시간
- ISR 진입 및 실행 오버헤드
- 실제 Clock 설정과 메모리 접근 시간

B는 0이 아니며 이번 Host Test에서는 실측하지 않았다.

실제 TC387QP에서 Worst-case Latency를 판단하려면 GPIO Toggle, Trace 등을 이용하여 측정해야 한다.

## 6. Timer 요청, ISR, Sensor 읽기의 시각 구분

```text
시간(ms)       10       20       30       35
               |        |        |        |
Timer 요청     IRQ      IRQ      IRQ      |
               |        |        |        |
ISR 기록       10       20       30       |
                                         |
Main 처리 ------------------------------>|
                                         |
                                    Snapshot 확보
                                         |
                                    Sensor read
                                         |
                                       Publish
```

위 그림은 ISR 요청이 지연 없이 기록되고 Main이 35ms에 실행되기 시작하는 개념적 예시이다.

실제 센서 읽기 시작 시점은 Snapshot 처리 시간 이후이며 정확한 Latency는 측정하지 않았다.

tick 10, 20, 30의 요청이 누적된 상태에서 Main이 35ms에 `collector_step()`을 실행하면 다음 Report를 생성한다.

```text
request_tick = 30
requests     = 3
coalesced    = 2
dropped      = 0
```

이때 `request_tick=30`은 마지막으로 수락한 ISR 요청 시각이며, 실제 센서 측정 시각은 아니다.

또한 `platform_timer_now()`를 Main에서 새로 읽지 않기 때문에 Main 실행 지연으로 요청 Timestamp가 변경되지 않는다.

## 7. Mailbox Overflow와 요청 병합

Mailbox 용량은 4이다.

Main이 요청을 가져가기 전에 tick 10, 20, 30, 40, 50, 60 요청에 대해 ISR이 각각 실행되었다고 가정한다.

| 요청 | pending | dropped | last_tick |
|---|---:|---:|---:|
| tick 10 | 1 | 0 | 10 |
| tick 20 | 2 | 0 | 20 |
| tick 30 | 3 | 0 | 30 |
| tick 40 | 4 | 0 | 40 |
| tick 50 | 4 | 1 | 40 |
| tick 60 | 4 | 2 | 40 |

최종 Mailbox 상태는 다음과 같다.

```text
pending   = 4
dropped   = 2
last_tick = 40
```

Main은 최신 센서값을 한 번 읽고 다음 Report를 생성한다.

```text
requests     = 4
coalesced    = 3
dropped      = 2
request_tick = 40
```

`coalesced`는 수락한 요청 중 추가로 병합된 개수이고, `dropped`는 Mailbox 용량 초과로 수락하지 못한 개수이다.

`dropped`는 `UINT32_MAX`에서 포화되도록 구현하여 정수 Wraparound를 방지하였다.

이는 실제 실행된 ISR의 소프트웨어 요청을 대상으로 한 정책이며, 하드웨어에서 처리되지 못한 Compare 이벤트를 모두 추적한다는 의미는 아니다.

## 8. 추가 Test와 검증 결과

### Test 1. IRQ Disabled 상태에서 여러 요청

IRQ를 차단한 뒤 tick 10, 20, 30 요청을 발생시켰다.

Host 모델에서는 IRQ가 차단되어 있을 때 하나의 Pending IRQ만 보류하며, 추가 요청은 기존 Pending 시각을 갱신한다.

IRQ 복원 전에는 ISR이 실행되지 않았고, 복원 후에는 마지막 tick 30 요청에 대한 ISR이 한 번 실행되는 것을 확인하였다.

이 결과를 통해 Software Mailbox의 용량 4와 하드웨어 IRQ Pending 모델의 저장 능력이 서로 다르다는 점을 확인하였다.

### Test 2. Publish 중 새로운 요청 발생

tick 10, 20 요청을 수락한 후 Main의 Publish 시점에 tick 30 IRQ를 주입하였다.

첫 Report는 다음 값을 유지하였다.

```text
requests     = 2
coalesced    = 1
request_tick = 20
```

새로운 tick 30 요청은 Mailbox에 남았고, 다음 `collector_step()`에서 처리되었다.

Sensor read와 Publish는 각각 총 2회 수행되었으며, ISR 내부 I/O와 IRQ Disabled 상태의 I/O는 발생하지 않았다.

### 전체 검증 결과

| 검증 단계 | 결과 |
|---|---|
| 수정 전 원본 Test | 422 checks, 106 failures |
| C 모듈 수정 후 Test | 422 checks, 0 failures |
| 추가 경계 Test 포함 | 452 checks, 0 failures |
| Demo 실행 | Exit code 0 |
| ISR 내 I/O | 0회 관찰 |
| IRQ Disabled 중 I/O | 0회 관찰 |

실행 증거는 `logs/before-test.log`, `logs/after-test.log`, `logs/student-test.log`, `logs/demo.csv`에 저장하였다.

## 9. 실제 Hardware 검증의 한계

이번 검증은 Host C11 단일 Thread 환경에서 수행하였다.

`platform_interleave()`와 Fake Platform을 통해 특정 시점의 IRQ 요청을 결정적으로 재현했지만, 실제 TC387QP의 다음 항목은 검증하지 않았다.

- 실제 STM Clock 및 Tick 변환
- MMIO 접근 및 Interrupt Router 설정
- ISR 진입 지연과 Worst-case Latency
- Critical section 실행 시간 B
- 실제 Sensor read 및 Publish 실행 시간
- 10ms 이내 다음 STM Compare 설정 가능 여부

특히 `IfxStm_increaseCompare()`와 같은 다음 Compare 갱신 방식은 기존 Compare 값 기준으로 주기를 유지하는 데 유용하지만, ISR 진입이 과도하게 늦어져 갱신 시점이 다음 Compare 시각을 지나면 기대한 주기 동작을 보장하기 어렵다.

따라서 실제 장비에서는 Timer 주파수와 Compare 설정, ISR 최악 실행 시간, Interrupt 차단 시간 및 Sensor 처리 지연을 함께 확인해야 한다.

## 10. 공식 참고 자료

- [TC38x Data Sheet Addendum, v1.9](https://www.infineon.com/assets/row/public/documents/10/49/infineon-tc38x-addendum-datasheet-en.pdf)
- [TC3xx User Manual Part 2, V2.0.0 — STM §27](https://www.infineon.com/assets/row/public/documents/10/44/infineon-aurix-tc3xx-part2-usermanual-en.pdf)
- [Infineon System Timer (STM) 공식 설명](https://documentation.infineon.com/aurixtc3xx/docs/wvt1713281569556)
- [AP32381 — TC3xx Startup and Initialisation](https://documentation.infineon.com/aurixtc3xx/docs/nyb1710229964455)
- [STM_Interrupt_1_KIT_TC387_TFT — 고정 공식 예제](https://github.com/Infineon/AURIX_code_examples/tree/6759d2845feaac0107b785fe6bc57ed785c3f4fc/code_examples/STM_Interrupt_1_KIT_TC387_TFT)
- [06회차 REFERENCES.md](https://github.com/zinee-u/embedded-just-for-fun/blob/main/weeks/week06/REFERENCES.md)

## 결론

이번 개선에서는 ISR 내부의 긴 I/O와 Delay를 제거하고, Main과 ISR 사이의 수집 요청을 Mailbox로 전달하도록 변경하였다.

또한 공유 데이터의 읽기와 초기화를 하나의 Critical section으로 보호하고 이전 IRQ 상태를 복원하여, 새 요청 유실과 IRQ 상태 변경 문제를 해결하였다.

Host Test 452개 검사를 모두 통과하였으며, Demo를 통해 요청 병합, Overflow 및 Snapshot 중 새로운 요청의 보존을 확인하였다.

다만 Host에서 검증한 논리적 정확성과 실제 TC387QP Hardware에서의 시간 요구 충족 여부는 구분해야 하며, 실제 적용 시 Clock과 Interrupt Latency 측정이 추가로 필요하다.