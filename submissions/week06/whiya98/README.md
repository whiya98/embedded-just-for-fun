# 06회차 Interrupt 수집 개선 Starter

TC387QP의 Timer Interrupt 설계를 참고하는 C11 Host 과제입니다. `make`, `make test`, `make demo` 순서로 실행합니다. 최초 Starter는 Build에 성공하지만 요구사항 Test에는 실패하도록 구성했습니다.

수정 파일은 `src/event_mailbox.c`, `src/timer_isr.c`, `src/collector.c`입니다. Public API와 제공 Host, 고정 Test를 바꾸지 않고 `tests/test_student.c`에 최소 2개의 Test를 추가합니다. `event_reset()`은 제공된 초기화 Helper로 유지합니다.

`include/mission.h`는 Mailbox와 Report 계약, `include/platform.h`는 Platform 계약입니다. `platform_interleave()`는 Snapshot 뒤·Clear 전에 정확히 1회 유지합니다. IRQ 금지 구간에서 도착한 Hook IRQ는 이전 상태가 Enabled로 복원된 뒤 실행됩니다.

`CONCURRENCY.md`, `BUILD_ENV.md`에 공식 근거·경쟁 순서·지연 분석·실행 결과를 기록합니다. `logs/test.log`는 Git 관리가 가능하며 Binary는 `build/`에서 제외합니다.

Host는 실제 C Thread나 TC387QP Firmware가 아닙니다. Main + 단일 비중첩 Timer ISR의 선점을 Hook으로 재현합니다. 실제 Register 접근, Timer/Vector/Clock 설정, 실제 Hardware 누락 주기 계수와 Latency 측정은 수행하지 않습니다. 상세 요구와 PR 경로는 상위 과제 README를 따릅니다.
