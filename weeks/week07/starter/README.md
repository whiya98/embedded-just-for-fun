# 07회차 TC387QP UART 기록·Parsing Starter

TC387QP의 ASCLIN UART 설계를 참고하는 C11 Host 과제입니다. `make`, `make test`, `make demo`로 실행합니다. 제공 Starter는 Build가 성공하고 요구사항 Test에는 실패합니다. 실패 Log와 코드를 근거로 문제를 직접 정의한 뒤 개선합니다.

수정 대상은 `src/uart_config.c`, `src/frame_codec.c`, `src/stream_rx.c`, `src/stream_tx.c`입니다. `include/mission.h`, `include/platform.h`, `host/`, `tests/test_mission.c`의 계약과 고정 Test는 유지하고 `tests/test_student.c`에 서로 다른 Test를 최소 2개 추가합니다. 초기화 Helper는 정상 상태 생성에 사용합니다.

## 먼저 남길 기록

1. 수정 전 `make`, `make test`, `make demo` 결과를 저장합니다.
2. `PROBLEM_DEFINITION.md`에 **현상 → 원인 가설 → 확인 근거 → 근본 원인 → 해결방안 → 검증**을 기록합니다. 관찰한 결과와 추정은 구분합니다.
3. `PROTOCOL.md`에 Wire format, RX 상태, TX 수용 정책과 전송량 계산을 작성합니다.
4. `BUILD_ENV.md`에 환경·명령·Exit·추가 Test를 남깁니다. Binary는 `build/`에 두고 제출하지 않습니다.

## API와 입력 계약

- `Record`: `seq=0..65535`, `value=0..4095`, `status=0..3`. `seq`는 순번 값이며, 중복·역전·Wrap을 별도 거부하는 계약은 없습니다.
- Wire format은 ASCII `S,<seq>,<value>,<status>\n`입니다. 숫자는 `0` 또는 선행 `0` 없는 양수의 decimal 표기입니다. 최대 길이는 LF 포함 15 byte입니다. 부호, 공백, CR, NUL, 추가 필드, 뒤쪽 문자와 잘못된 Prefix는 거부합니다.
- `frame_parse(line,len,out)`은 **LF를 제외한 Byte span**을 받습니다. NUL termination을 가정하지 않습니다. 실패하면 `false`이며 `*out`을 보존합니다. `line`/`out`이 NULL이거나 `len>=32`이면 실패합니다.
- `frame_encode(record,out,capacity,out_len)`은 LF를 포함한 Byte span을 씁니다. NUL terminator는 쓰지 않습니다. 성공하면 `true`, `*out_len`은 실제 byte 수입니다. 공간 부족·범위 오류·NULL 인자는 `false`이며 출력 Buffer와 `*out_len`을 보존합니다. `seq`는 이미 `uint16_t` 범위로 표현됩니다.
- `stream_rx_feed()`는 임의로 나뉘거나 합쳐진 Byte chunk를 입력받고 **이번 호출에서 수용한 완전한 frame 수**를 반환합니다. LF 전에는 Record를 반영하지 않습니다. 마지막 정상 결과는 `have_last/last`에 보존하며 오류 누적 Counter는 없습니다.
- RX Buffer는 32 byte이고 LF 전 최대 31 byte를 저장합니다. 32번째 일반 byte가 오면 해당 line 전체를 LF까지 버립니다. 문법 검사는 LF 도착 시점에 수행하며 `discarding`은 용량 초과 상태에만 사용합니다. 잘못된 line도 LF가 끝난 뒤 다음 line부터 복구합니다.
- `stream_tx_queue()`는 유효한 frame 전체를 Pending Buffer 하나에 수용하거나 거부합니다. Busy이면 `false`이고 기존 Pending frame을 보존합니다. 32 byte Buffer가 여러 frame의 Queue를 뜻하지는 않습니다.
- `stream_tx_pump()`는 Pending일 때 `platform_uart_write()`를 한 번 호출하고 이번에 수용한 prefix만 소비합니다. 수용 수가 0이어도 반환하며, 다음 호출에서 남은 부분을 이어갑니다. 모두 수용된 뒤에만 Idle이 됩니다. 반복 Pump로 중복 전송하지 않습니다.
- Platform write는 `0..요청길이`를 반환하고 해당 prefix를 내부로 복사하며 순서를 보존합니다. 이 반환값은 실제 Pin의 마지막 Stop bit 전송 완료를 뜻하지 않습니다. Test의 `fake.wire`는 수용된 순서의 관찰 Buffer입니다.

모든 non-NULL Pointer는 표시된 길이만큼 읽기/쓰기가 가능해야 합니다. 서로 다른 입력·출력 객체는 겹치지 않고 State 구조체는 `init` 이후 API로만 변경합니다. `stream_rx_feed(rx,NULL,0)`은 정상적인 무입력 호출이며 상태를 보존합니다. `rx==NULL` 또는 `bytes==NULL && len>0`이면 0과 상태 보존입니다. NULL 초기화/Pump는 아무 일도 하지 않고 NULL TX 조회는 Idle입니다.

## Host 범위

API 호출은 Main의 단일 Thread에서 직렬화합니다. 이 과제에서 ISR/Multicore/Thread 간 동시 접근을 실행하지 않습니다. `platform_uart_apply/write`는 과제의 Host 추상화이며 TC387QP Register나 실제 iLLD API가 아닙니다. Host는 Baud 오차, 실제 FIFO/Interrupt, Pin MUX, Framing error, Parity error, Overrun, 실시간 송신 지연을 재현하지 않습니다. 실제 Board에서는 ASCLIN 설정·Clock·Pin과 iLLD 반환값 계약을 별도로 확인해야 합니다. 공개 Test만 통과했다고 Board 검증을 완료한 것으로 기록하지 않습니다.
