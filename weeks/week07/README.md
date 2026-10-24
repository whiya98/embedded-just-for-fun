# 07회차 TC387QP UART 기록·Parsing 개선 요청서

[문제 풀이 접근법](APPROACH.md) · [공식 근거](REFERENCES.md) · [원본 관찰](baseline/README.md) · [PR 제출 안내](SUBMISSION.md)

## 같은 센서 기록이 전달 방식에 따라 달라집니다.

> 요청 범위: UART 115200 / 8N1에서 센서 기록의 출력·수신·검증을 일관되게 처리해 주세요.

개발팀은 TC387QP 수집 기능에 UART 기록과 수신 데이터 해석 기능을 연결하고 있습니다. 하나의 기록을 한 번에 전달하는 경우와 여러 번 나누어 전달하는 경우, 여러 기록이 함께 도착하는 경우에 결과가 달라집니다. 송신이 혼잡할 때의 기록 보존 여부도 확인이 필요합니다.

아래 요구 조건과 제공된 C 코드를 기준으로 현상을 재현해 주세요. 먼저 현상·원인 가설·해결방안을 직접 정의한 뒤 개선 코드와 검증 근거를 Pull Request로 제출해 주세요. 원인 가설은 실제 Test로 확인한 원인과 구분해 기록합니다.

### 학습 목표

| 목표 | 이번 과제에서 남길 증거 |
| --- | --- |
| UART 설정과 송수신 흐름을 확인하고 기록 형식을 정한다. | TC387QP의 ASCLIN 근거, 설정 표, 메시지 규격과 흐름도. |
| 센서 값·순번·상태를 출력하고 수신 데이터를 Parsing한다. | 네 C Module의 개선과 동일 Record의 Encode·Parse 검증. |
| 잘림·오류 입력과 기록량의 영향을 검토한다. | 재현 Log, 현상–원인–해결방안, 경계 Test와 처리량 계산. |

> 납품물: 개선한 C 코드, 직접 정의한 문제 목록, 공식 근거, 추가 Test와 실행 Log. 이번 작업은 Host에서 계약을 검증하는 과제이며 실제 Board 검증과 구분합니다.

## TC387QP의 사실과 이번 제품의 정책을 구분합니다.

| 근거 | 확인할 내용 |
| --- | --- |
| TC38x Datasheet / Addendum | TC387QP 제품과 Package를 확인합니다. 실제 Board의 부품 표기와 배선을 따로 확인합니다. |
| TC3xx User Manual의 ASCLIN | ASC 모드, Clock과 Baud rate 생성, UART Frame, TX/RX FIFO와 Error flag를 찾습니다. |
| TC387 TFT 공식 UART 예제 / iLLD | initSerialInterface의 Clock·Baud·Pin·FIFO·ISR 설정과 IfxAsclin_Asc_read/write 계약을 추적합니다. |
| 이번 요청서 | 115200 / 8N1, ASCII 문법, 용량, 거부·복구 정책, 한 개 Pending TX는 제품 요구 조건입니다. |

실행 환경은 C11 Host입니다. platform_uart_apply/write는 관찰 가능한 Test 대역이며 TC387QP의 MMIO Register나 iLLD를 구현한 것이 아닙니다. CPU0의 Main에서 모든 API를 직렬 호출합니다. 여러 Core·DMA·ISR의 동시 접근은 이번 필수 수정 범위에 포함하지 않습니다.

실제 적용 시에는 정확한 Board·Pin 경로·Clock·iLLD 버전·Interrupt 설정을 별도로 확인해야 합니다. 제공 예제의 Board Pin을 다른 TC387QP Board의 배선으로 그대로 단정하지 않습니다.

> Host가 검증하는 것은 설정 요청값과 Byte 처리 계약입니다. 실제 Baud 오차, Pin 전압, 신호 품질, Hardware FIFO Overrun, ISR 지연이나 Board 동작을 측정한 결과가 아닙니다.

[Infineon TC387 TFT 공식 예제 모음](https://github.com/Infineon/AURIX_code_examples/tree/6759d2845feaac0107b785fe6bc57ed785c3f4fc/code_examples)

정확한 문서 위치와 Source 함수는 함께 제공한 REFERENCES.md를 사용합니다. 문서명·절·함수·확인한 사실을 PR에 남겨 주세요.

## 출력 형식과 입력 허용 범위를 고정합니다.

```text
S,<seq>,<value>,<status>

예: S,42,1200,1\n
최대: S,65535,4095,3\n
```

| 항목 | 계약 |
| --- | --- |
| UART | 115200 bit/s, Data 8 bit, Parity 없음, Stop 1 bit. TX와 RX를 모두 사용. |
| seq / value / status | seq 0..65535, value 0..4095, status 0..3. seq=0도 유효합니다. 중복·역순 seq 거부 정책은 없습니다. |
| 숫자 문법 | ASCII 십진수. 0 또는 0으로 시작하지 않는 숫자열. 부호·공백·선행 0·빈 필드·소수점은 거부. |
| 구분과 종결 | 대문자 S, 쉼표 3개, 마지막 LF(0x0A). CR·NUL·추가 필드·뒤쪽 쓰레기 문자는 거부. |
| 길이 | 최대 정상 Wire Frame은 LF 포함 15 Byte. C 문자열의 NUL은 Wire에 보내지 않습니다. |
| 정상 수신 반영 | LF가 도착한 뒤 전체 문법과 범위를 만족한 Record만 수용합니다. 거부 시 마지막 정상 Record를 보존합니다. |

> 문서의 \n은 LF 한 Byte를 뜻합니다. 문자 \와 n 두 개를 보내는 규격이 아닙니다. 수신 API는 주소와 길이로 Byte를 받으므로 NUL을 입력 끝으로 해석하지 않습니다.

## 입력 경계와 송신 혼잡에서도 계약을 유지합니다.

| 영역 / API | 요구 동작 |
| --- | --- |
| uart_configure | 115200 / 8N1, TX·RX enabled로 Platform에 설정을 요청합니다. |
| frame_parse<br>frame_encode | Parse는 LF를 제외한 line과 길이를 받습니다. 전체 유효성을 확인한 뒤 Record를 반영합니다. Encode는 LF를 포함한 Frame을 만들며 실패 시 출력과 길이를 보존합니다. |
| stream_rx_feed | 임의의 Chunk 분할과 합쳐진 입력을 처리합니다. 호출 결과는 이번 호출에서 수용한 Frame 수입니다. have_last / last로 마지막 정상 Record를 유지합니다. |
| RX 용량 / 복구 | 32 Byte Buffer에 LF 전 최대 31 Byte를 보관합니다. 초과한 줄은 LF까지 통째로 버립니다. 같은 Chunk에 뒤따르는 정상 줄부터 다시 수용합니다. |
| stream_tx_queue | 32 Byte Pending Frame 하나를 사용합니다. 유효한 Record 전체 수용 시 true. Busy·잘못된 Record는 false이며 기존 Pending Frame을 보존합니다. |
| stream_tx_pump | Pending이면 Platform write를 한 번만 호출합니다. 수락한 Byte 수만 진행하고, 0이면 보존합니다. 전체 수락 후에만 Busy를 해제합니다. |

변경 대상은 src/uart_config.c, frame_codec.c, stream_rx.c, stream_tx.c입니다. 제공 Header·Host·고정 Test의 계약은 유지하고 private Helper는 추가할 수 있습니다. 구조체·NULL·출력 용량의 세부 계약은 include/mission.h와 starter/README.md를 따릅니다.

> 한 번의 write 수락은 Application Buffer에서 Driver로 넘어간 양입니다. 마지막 Stop bit가 Pin을 떠났거나 상대가 수신했다는 뜻은 아닙니다. 무한 대기 Loop로 혼잡을 숨기지 않습니다.

## 관찰값부터 수집하고 원인 가설을 세워 주세요.

| 재현 입력 / 순서 | 기대 결과 |
| --- | --- |
| 정상 한 줄: S,1,100,0\n | 1개 수용, last={1,100,0}. |
| S,1, → 100,0 → LF 순서의 3회 feed | 앞의 두 호출은 0개. 마지막 호출에서 정확히 1개 수용. |
| S,1,100,0\nS,2,200,1\n 한 번에 feed | 2개 수용, 마지막 Record는 {2,200,1}. |
| S,65536,100,0\n 뒤에 정상 줄 | 범위 초과 줄은 거부. 다음 정상 줄은 수용. |
| S,01,100,0\n / S,1,100,0X\n | 둘 다 거부. 기존 last는 그대로 유지. |
| LF 없는 32 Byte 이상 + 정상처럼 보이는 suffix + LF | 줄 전체 거부. 다음 LF-종결 정상 줄에서 복구. |
| TX A queue → Busy 중 B queue | A true, B false. A의 미송신 부분을 그대로 보존. |
| TX write 수락량 2 → 0 → 나머지 | 첫 2 Byte 이후 상태 유지. 다시 진행해 A가 정확히 한 번 전달. |

make test와 make demo의 원본 Log를 먼저 저장합니다. 각 기대 결과 옆에 실제 값·Test 이름·환경을 기록하세요. 실패한 Check 수는 근본 원인의 개수가 아닙니다. 비슷한 증상은 같은 원인인지 별도 원인인지 검증해야 합니다.

```text
제공 원본 Demo 관찰 (2026-10-24, macOS Host)
RX: [S,1,23] [,0\nS,2,45,1\n]
    accepted=0+0, have_last=0, last=1/23/0
TX: 최대 Frame / 수락 한도 3,0,2,32 / pump 4회
    accepted=3, calls=1, busy=0, bytes=S,6
```

> 위 표의 “기대”는 요구사항입니다. 실행 전 예상값을 실제 관찰값으로 적지 않습니다. Hardware 통신이 안 된다는 관찰과 Host 설정값 불일치도 서로 다른 증거입니다.

## 현상–원인–해결방안을 자신의 문장으로 정의합니다.

| 기록 단계 | 직접 작성할 질문 |
| --- | --- |
| 1. 현상 | 어떤 초기 상태와 Byte 입력에서 재현되는가? 기대값과 실제값은 무엇인가? 전달 Chunk 경계도 같은가? |
| 2. 원인 가설 | 어떤 조건·상태 변화가 현상을 설명하는가? 경쟁 가설은 무엇인가? 가설이 틀렸다면 어떤 입력에서 드러나는가? |
| 3. 확정 원인 | 어느 함수·분기에서 어떤 불변식이 깨지는가? 관찰값과 연결되는 실행 순서는 무엇인가? |
| 4. 해결방안 | 지켜야 할 상태와 변경 규칙은 무엇인가? 대안과 선택 근거는 무엇인가? |
| 5. 검증 | 같은 재현 입력에서 어떻게 달라졌는가? 새 경계 Test는 어떤 가설을 검증하는가? 남은 한계는 무엇인가? |

```text
P01
현상: [초기 상태 / 입력 / 기대 / 실제 / 재현 명령]
원인 가설: [가설 A / 가설 B / 구별할 Test]
확정 원인: [파일·함수 / 깨진 규칙 / 증거]
해결방안: [상태·알고리즘 / 대안 / 선택 이유]
검증: [수정 전후 / 추가 경계 / 남은 한계]
```

“통신이 이상하다”, “Buffer 문제다”, “Test가 통과했다”만으로 문제를 정의하지 않습니다. 현상은 관찰 가능한 값으로, 원인은 그 값을 만든 동작으로, 해결방안은 재발하지 않도록 지킬 규칙으로 적습니다.

> PROBLEM_DEFINITION.md를 수정 전에 먼저 작성하고, Test 후 가설을 갱신해 주세요. 문제 풀이 접근법의 원인 해설은 자신의 분석을 작성한 뒤 대조하는 자료로 사용합니다.

## 코드가 맞아도 전송량은 따로 검토해야 합니다.

모든 Frame이 최대 길이인 상황을 가정합니다. 100 Hz로 기록할 때와 1000 Hz로 기록할 때 필요한 전송량을 비교하세요. 아래 숫자는 제품 분석 조건이며 실제 Board 측정값이 아닙니다.

| 검토할 항목 | 계산·검증에 포함할 내용 |
| --- | --- |
| Frame 비용 | 8N1의 Start / Data / Parity / Stop을 세고, 15 Byte Frame의 Wire 전송 시간을 계산. |
| 100 Hz / 1000 Hz | 초당 Byte·bit 수와 115200 bit/s 대비 사용률. 지속 가능한 경우와 불가능한 경우를 구분. |
| Busy 처리 | 거부된 Record를 호출자가 어떻게 기록·재시도·폐기할지 정책 제안. Buffer 확대로 해결되는 상황과 안 되는 상황 구분. |
| 추가 Test 최소 2개 | 서로 다른 경계에서 예상값을 먼저 작성. 예: 모든 Chunk 절단점, 길이·범위 경계, 0 진행 뒤 재개, 오류 뒤 복구. |
| 회귀 검증 | 원본 실패와 개선 후 결과를 같은 입력으로 비교하고 기존 정상 동작이 유지되는지 확인. |

고정 Test를 약화하거나 API·Host의 동작을 바꾸어 통과시키지 않습니다. 모든 실패를 원인 목록과 연결하고, 아직 실패하지 않은 경계도 코드 계약에 따라 검토하세요. 정상 입력을 반복하는 것만으로 추가 Test를 채우지 않습니다.

> ASCLIN Hardware FIFO, iLLD Software FIFO, Application Pending Buffer는 별개의 저장 공간입니다. 용량만 크게 잡는 것이 무손실·무지연을 보장하지 않습니다.

## 코드와 문제 정의를 한 PR에 제출합니다.

| 위치 | 내용 |
| --- | --- |
| src/ + tests/ | 네 C Module 개선, 고정 Test와 직접 추가한 Test. include/·host/·Makefile도 함께 복사. |
| PROBLEM_DEFINITION.md | 현상–원인–해결방안, 가설 구별 Test, 근본 원인별 검증 표. |
| PROTOCOL.md | 문법, RX 상태·복구 정책, TX 진행·Busy 정책, 100 Hz / 1000 Hz 계산, 공식 근거. |
| README.md<br>BUILD_ENV.md<br>logs/ | 개선 요약, 실제 환경·명령·Exit code, 원본/개선 Test와 Demo 결과. |

```text
submissions/week07/YOUR_GITHUB_ID/
  src/             include/         host/
  tests/           Makefile         .gitignore
  README.md        PROTOCOL.md      BUILD_ENV.md
  PROBLEM_DEFINITION.md             logs/
```

1. 원본 Starter를 자신의 제출 폴더로 복사하고 Branch를 만듭니다.
2. 원본 Log와 문제 정의 초안을 저장한 뒤 구현·분석·추가 Test를 진행합니다.
3. 정상·잘림·오류·혼잡 조건의 검증을 끝내고 PR_BODY.md를 실제 값으로 채웁니다.
4. 자신의 Fork에서 zinee-u/embedded-just-for-fun:main으로 PR을 만듭니다.

> [문제 풀이 접근법](APPROACH.md)에서 기술 개념·공식 근거·모든 근본 원인을 확인할 수 있습니다. 먼저 자신의 문제 정의를 작성한 뒤 대조하고, [제출 안내](SUBMISSION.md)에 따라 자기 제출 폴더로 PR을 보내 주세요. 제출 마감일은 별도 안내합니다.


## 변경 기록

- 2026-10-25: 07회차 과제와 문제 풀이 접근법 Markdown을 최초 공개했습니다. PDF와 완성된 참고 구현은 포함하지 않습니다.
