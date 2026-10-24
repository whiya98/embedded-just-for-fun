# 07회차 UART Protocol·처리량 기록

## Wire format과 실제 Hardware 근거

| 확인 항목 | 공식 문서·Version·위치 | 확인한 조건 | 이번 구현 판단 | 검증 수단 |
| --- | --- | --- | --- | --- |
| TC387QP의 ASCLIN UART 지원 | | | | |
| ASCLIN Clock와 Baud 생성 | | | | |
| RX/TX enable 및 Pin MUX | | | | |
| 실제 iLLD Read/Write의 수용 수·Timeout 계약 | | | | |
| Hardware 오류와 Application Parser 오류의 구분 | | | | |

- 사용한 Board 및 TX/RX Pin (Host만 실행했다면 미실행으로 표기):
- Frame 문법, 각 필드 범위, Canonical decimal의 의미:
- LF/CR/NUL 처리 규칙과 선택 이유:
- 하나의 API 호출과 하나의 frame이 같지 않다는 재현 입력:
- 부분 RX / line overflow / 오류 뒤 복구 상태도:
- `have_last`와 `last`의 의미 및 잘못된 입력 전후 보존값:
- TX Pending, Busy 거부, 부분 수용·0 수용을 처리하는 상태도:
- Platform 수용 완료와 실제 Pin 송신 완료의 차이:

## 전송량 계산

조건은 115200 baud, 8N1, 최대 frame 15 byte입니다. UART 8N1의 한 byte가 차지하는 Wire bit 수를 Start/Data/Parity/Stop으로 나누어 계산합니다.

| 입력률 | Frame당 byte | Byte당 Wire bit | 요구 bit/s | UART 점유율 | 안정적으로 지속 가능한가? |
| --- | ---: | ---: | ---: | ---: | --- |
| 100 frame/s | 15 | | | | |
| 1000 frame/s | 15 | | | | |

- 최대 frame 하나의 이론적 전송 시간:
- 최대 frame 기준 이론적 frame/s 한계와 가정:
- Pending Buffer를 크게 하면 지속적인 입력률 초과가 해결되는가:
- Busy 때 상위 호출자가 선택할 재시도/간소화/수집률 조정 정책:
- Logging 작업을 기다리는 Loop로 만들지 않은 이유:
- Host가 증명한 사실 / 실제 Board 측정이 필요한 사실:

실제 시스템의 상위 호출자 정책, HW FIFO 처리, Interrupt Priority와 측정 시간은 이 과제의 Host Stub 구현과 구분합니다. 초 단위 처리량은 이론적 상한이며 실제 유지 가능한 처리량이라고 단정하지 않습니다.
