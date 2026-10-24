# 07회차 TC387QP UART 기록·Parsing 개선 요청서 문제 풀이 접근법

[과제](README.md) · [공식 근거](REFERENCES.md) · [원본 관찰](baseline/README.md) · [PR 제출 안내](SUBMISSION.md)

## 개념과 근거를 연결해 문제를 정의합니다.

| 목표 | 근거 → 판단 → 검증 |
| --- | --- |
| UART 설정과 기록 형식 | Datasheet·ASCLIN Manual·공식 초기화 예제 → Hardware 설정과 제품 문법 구분 → 설정값 관찰과 형식 Test. |
| 출력과 Parsing | Wire Byte와 C 객체의 차이 → 길이·문법·범위·반영 시점 결정 → 정상·경계·오류의 Encode/Parse. |
| 잘림·오류·기록량 | Byte Stream·부분 진행·처리량 → RX 상태와 TX 소유권·복구 정책 → 분할/연속 입력·혼잡 Test와 전송 예산. |
| 문제 정의 | 관찰된 실패와 코드 실행 순서 → 반증 가능한 원인 가설 → 최소 재현·수정 전후 비교·회귀 Test. |

먼저 [문제 정의 양식](starter/PROBLEM_DEFINITION.md)을 자신의 관찰로 채웁니다. 이 문서는 필요한 기술 개념, 공식 자료에서 확인할 근거, 분석 예시, 모든 근본 원인 검토 순으로 읽을 수 있습니다. 원인 문장을 그대로 옮기는 것보다 자신의 Test로 그 문장이 성립하는 조건을 입증해야 합니다.

> 요구사항은 센서 기록의 Byte 표현을 안정적으로 다루는 것입니다. 이번 seq는 Record 식별 값이며 ACK·재전송·중복 제거를 제공하지 않습니다. 송신 수락을 상대의 수신 완료로 해석하지 않습니다.

## 관찰과 원인 가설은 다른 문장으로 씁니다.

| 단계 | 작성 예시: 동일 Frame의 Chunk 분할 |
| --- | --- |
| 현상 | S,1,100,0\n 한 번과 S,1, / 100,0 / LF 세 번의 입력을 비교한다. 기대 Record는 같지만 원본 결과가 다른지 실제 Log로 확인한다. |
| 가설 A | 호출이 끝날 때 미완성 데이터를 보관하지 않는다면, 중간에 나눈 경우만 실패할 것이다. |
| 가설 B | 문법 또는 숫자 검증 자체의 문제라면, 같은 완성 line을 frame_parse에 직접 주어도 실패할 수 있다. |
| 구별 Test | 완성 line의 Parse를 먼저 확인한 뒤, 모든 Byte 절단점에서 두 번 feed한다. 실패 절단점과 호출 뒤 상태를 기록한다. |
| 확정 원인 | 가설과 일치하는 함수·상태 변화를 찾아 Chunk 종료와 Frame 완료를 혼동했는지 입증한다. |
| 해결방안 | LF까지 미완성 line을 보관하고 완성 시 검증한다. 같은 줄의 모든 절단점에서 결과가 같아야 한다. |

“Buffer를 추가했다”는 구현 설명일 뿐 원인 설명은 아닙니다. 보존해야 하는 상태가 무엇인지, 언제 초기화해야 하는지, 잘못된 줄 이후 어디에서 복구할지를 함께 설명해야 합니다.

> 위 현상 문장은 검증 절차의 예시입니다. 실제 결과와 확정 원인은 제공 코드의 실행 Log·함수·분기로 뒷받침합니다. 같은 원인의 여러 증상은 묶고, 서로 다른 원인을 하나의 “통신 문제”로 합치지 않습니다.

## CPU Clock과 UART Baud는 같은 값이 아닙니다.

ASCLIN은 TC3xx의 통신 주변장치이며 ASC 모드로 비동기 직렬 통신을 수행합니다. UART는 공유 Clock 선 없이 합의한 Baud와 Frame 형식으로 Byte를 전송합니다. TX/RX Pin 경로와 양쪽 설정이 맞아야 올바르게 해석할 수 있습니다.

1. Datasheet / Addendum에서 정확한 TC387QP 제품과 Package를 확인합니다.
2. ASCLIN Manual에서 입력 Clock 선택, Prescaler·Fractional divider와 Bit Timing 경로를 추적합니다.
3. 공식 예제 initSerialInterface에서 115200, Oversampling, Pin, FIFO Buffer, ISR 설정을 연결합니다.
4. iLLD의 initModuleConfig 기본값과 실제 적용 값을 구분하고 read/write의 반환 계약을 확인합니다.

| 혼동하기 쉬운 값 | 판단 |
| --- | --- |
| CPU 최대 동작 주파수 | CPU의 제품 한계이며 현재 ASCLIN 입력 Clock이나 Baud를 뜻하지 않습니다. |
| 115200 bit/s | 이번 제품의 Baud 설정입니다. 실제 분주 결과와 오차는 선택 Clock·설정으로 확인합니다. |
| Board Pin | 공식 KIT 예제의 연결 예일 뿐, 임의 Board의 배선을 보증하지 않습니다. |
| Host 설정 Test | 요청한 값의 정확성만 확인합니다. 실제 Clock·Pin·전압의 정상 동작은 입증하지 않습니다. |

근거: TC3xx ASCLIN §36, 공식 TC387 UART 예제와 IfxAsclin_Asc_initModuleConfig. 정확한 URL·위치는 REFERENCES.md에 정리합니다.

## 8N1의 Frame과 메시지의 Frame을 구분합니다.

```text
UART 1 Byte: Start(1) + Data(8) + Parity(0) + Stop(1)
Application: S,42,1200,1 + LF

ASCLIN RX FIFO -> Driver Buffer -> Chunk -> Line Parser
Record -> Encoder -> Pending TX -> Driver -> TX Pin
```

UART Hardware가 다루는 Frame은 Start/Stop을 포함한 Byte 단위입니다. 이 과제의 Application Frame은 S와 숫자 필드, 쉼표, LF로 구성된 여러 Byte입니다. Hardware가 Byte를 정상 수신했다는 사실만으로 한 줄의 형식과 값까지 맞다고 판단할 수 없습니다.

| 저장 공간 | 역할과 한계 |
| --- | --- |
| ASCLIN Hardware FIFO | 송수신 Byte를 임시 저장합니다. 공통 ASCLIN의 TX/RX FIFO는 각각 16 Byte이며 메시지 경계를 만들어 주지 않습니다. |
| iLLD Software FIFO | ISR과 Application 사이의 Byte 처리량·서비스 지연을 흡수합니다. 설정한 용량과 실제 read/write 결과를 확인해야 합니다. |
| RX line Buffer | LF 전의 Byte를 보존합니다. 여기서는 32 Byte 배열에 최대 31 Byte의 line을 허용합니다. |
| Pending TX Buffer | 전체 Frame의 소유권을 유지합니다. write가 수락한 부분과 남은 부분을 나눠 기록합니다. |

> 같은 32 Byte라도 RX line 한계와 TX Pending 용량은 서로 다른 계약입니다. 정상 Wire 최대 15 Byte, Parse line 최대 14 Byte, NUL 포함 C 문자열 저장 공간은 별개의 길이입니다.

## read 한 번은 메시지 하나를 의미하지 않습니다.

UART에서 도착한 Byte 순서는 유지되지만 Application이 읽는 Chunk 크기는 처리 시점과 Driver 상태에 따라 달라질 수 있습니다. 하나의 line이 여러 호출로 나뉘거나, 한 호출에 여러 line과 다음 line의 일부가 함께 들어올 수 있습니다.

```text
입력 A: [S,1,100,0\n]
입력 B: [S,1,] [100,0] [\n]
입력 C: [S,1,100,0\nS,2,200,1\nS,3,]

정상 상태: Byte 축적 -> LF -> 전체 검증 -> 초기화
폐기 상태: Byte 무시 -> LF -> 정상 상태 복귀
```

| 상태 / 입력 | 지켜야 하는 동작 |
| --- | --- |
| 정상 / LF 아님 | 용량 안에서 누적하고 다음 호출까지 길이와 내용을 유지. |
| 정상 / LF | 현재 line 전체를 검증하고 유효할 때만 수용. 이후 같은 Chunk의 다음 Byte도 계속 처리. |
| 용량 초과 | 현재 줄을 폐기 상태로 전환. 용량만 0으로 만들고 suffix를 새 줄로 읽지 않음. |
| 폐기 / LF | 폐기 상태를 종료. 그 다음 Byte부터 새로운 line을 받음. |

LF 전 32번째 Byte가 들어와 31 Byte 한계를 넘은 뒤 S,2,200,1처럼 정상 문법의 suffix가 나온다고 해도 앞부분과 같은 줄입니다. LF 이전에 복구하면 잘못된 입력의 뒷부분을 새 정상 Record로 만들 수 있습니다.

> 이번 API의 discarding은 용량 초과를 나타내는 상태입니다. 문법은 LF가 도착했을 때 검증합니다. 조기 문법 검사 설계도 가능하지만 제공 상태·고정 Test와 다른 확장 설계이므로 이번 제출에서는 계약을 유지합니다.

## 숫자로 변환된다는 사실만으로 유효하지 않습니다.

| 검사 층 | 이유와 예 |
| --- | --- |
| 문법 | S 접두부, 필드 수, 쉼표 위치, 전체 소모 여부를 확인합니다. 1X를 숫자 1로 받아들이면 안 됩니다. |
| Canonical decimal | 0은 허용, 00·01·+1·-1·공백은 거부합니다. 표현을 하나로 고정하는 제품 정책입니다. |
| 범위 | seq≤65535, value≤4095, status≤3를 좁은 정수형으로 변환하기 전에 확인합니다. |
| 메모리 경계 | line은 NUL-종결 문자열이라는 보장이 없습니다. 항상 전달된 len 안에서만 접근합니다. |
| 전체 검증 | 마지막 필드까지 확인하기 전에 out을 고치지 않습니다. 실패 시 마지막 정상 상태를 보존합니다. |

```text
digit = byte - '0';
if (acc > limit / 10 ||
    (acc == limit / 10 && digit > limit % 10)) {
    reject;
}
acc = acc * 10 + digit;
```

위 식은 곱셈·덧셈 전에 상한을 확인하는 원리입니다. 문자 종류 검사와 필드 구분·선행 0 검사는 따로 필요합니다. strtoul을 사용하더라도 부호·공백 허용 여부, end pointer, 범위와 오류 상태를 추가로 확인해야 합니다.

> uint16_t에 넣은 뒤 65535 이하인지 검사하면 이미 값이 줄어든 뒤일 수 있습니다. 변환 성공과 제품의 입력 유효성은 같은 판단이 아닙니다.

## 성공할 때 한 번만 결과를 반영합니다.

Parse 과정은 임시 Record에서 진행하고 전체 검증이 성공할 때만 호출자의 out에 복사합니다. 실패한 입력 때문에 seq만 새 값이고 value는 이전 값인 부분 결과가 남아서는 안 됩니다. RX의 마지막 정상 Record도 이 성공 여부를 기준으로 갱신합니다.

```text
입력 -> 구조 검사 -> 숫자·범위 검사 -> 임시 Record
모두 성공 -> *out = temporary
실패     -> 기존 *out 유지

Record -> 유효성 검사 -> 임시 Wire Frame
용량 충분 -> 출력 Byte와 out_len 반영
실패     -> 출력과 out_len 모두 유지
```

C 문자열을 내부에서 사용한다면 최대 Wire 15 Byte 외에 NUL 한 Byte가 더 필요합니다. 반면 주소와 길이로 보내는 Wire Buffer의 외부 계약은 NUL을 전송하지 않습니다. 어떤 길이가 Wire 길이이고 어떤 길이가 문자열 저장 용량인지 이름과 Test로 구분합니다.

snprintf의 반환값은 정상일 때 실제 Buffer에 남긴 길이가 아니라 충분한 공간이 있었다면 기록했을 길이입니다. 반환값이 음수이거나 capacity 이상이면 그대로 전송 길이로 쓰면 안 됩니다. 임시 Buffer와 충분한 용량 확인 후 최종 출력을 반영합니다.

> 실패 시 출력 보존은 “에러면 false”와 별개의 계약입니다. out과 out_len에 Sentinel을 넣고 오류 입력·작은 용량을 준 뒤 값이 그대로인지 확인해야 합니다.

## 수락한 만큼만 진행하고 나머지는 보존합니다.

```text
Pending A = [S , 1 , 1 0 0 , 0 LF]
write 수락: 2 Byte -> offset = 2
write 수락: 0 Byte -> offset = 2, busy 유지
다음 pump: A의 offset부터 남은 Byte 요청
전부 수락: busy 해제, 다음 Record 수용 가능
```

Backpressure는 소비자가 지금 모든 데이터를 받아들일 수 없음을 생산자에게 알리는 흐름입니다. 이번 API는 Busy일 때 새 Record를 거부하고 false를 반환합니다. 호출자는 그 결과로 재시도·폐기·집계 정책을 선택해야 하며, 거부를 수락으로 기록해서는 안 됩니다.

| 잘못된 판단 | 올바른 계약 |
| --- | --- |
| write를 호출했으므로 송신 완료 | 요청량과 실제 수락량을 구분. 수락한 Byte 수만 offset 증가. |
| 0이면 데이터를 버리고 다음으로 | 진행이 없는 상태를 보존하고 다음 Main 기회에 다시 pump. |
| Busy여도 최신값으로 덮기 | 이 과제는 기존 Frame 보존. Queue가 false이면 새 Frame은 수락하지 않음. |
| 끝날 때까지 while 반복 | pump 한 번에 write 한 번. Driver가 계속 0을 반환해도 Main을 묶지 않음. |

> iLLD IfxAsclin_Asc_write의 bool과 *count를 함께 확인합니다. false라도 일부 Byte가 진행될 수 있습니다. Host Platform의 size_t 반환 계약에 연결하는 Target Adapter가 별도로 필요합니다.

## 지속 과부하는 Buffer만 키워 해결할 수 없습니다.

| 항목 | 계산 결과 |
| --- | --- |
| 8N1 비용 | 1 + 8 + 0 + 1 = 10 bit/Byte. |
| 이상적인 Payload 처리량 | 115200 / 10 = 11520 Byte/s. |
| 최대 Frame 15 Byte | 150 bit / 115200 = 약 1.302 ms. 이상적 상한 768 Frame/s. |
| 100 Hz | 1500 Byte/s = 15000 bit/s, 약 13.02% 사용. 나머지 비용·서비스 지연은 별도 검토. |
| 1000 Hz | 15000 Byte/s = 150000 bit/s, 약 130.21% 사용. 무손실로 지속 전송 불가. |

유한 Buffer는 순간 Burst나 일시적인 서비스 지연을 흡수할 수 있습니다. 생산률이 소비률보다 계속 높으면 결국 가득 찹니다. 기록 주기 증가(기록률 감소), 기록 축약, 승인된 Baud 변경, 손실 허용·집계 등의 제품 정책을 검토해야 합니다. 이번 과제에서는 115200 설정을 바꿔 문제를 우회하지 않습니다.

Parity·Framing·Overrun은 Hardware 수신 경로의 오류입니다. 8N1에서는 Parity를 사용하지 않습니다. 쉼표 누락·범위 초과·너무 긴 line은 Application 문법 오류입니다. LF 기반 Parser가 모든 전송 오류를 검출하지는 않습니다. 숫자가 다른 유효 숫자로 바뀌면 이 규격으로는 통과할 수 있습니다.

> CRC·ACK·재전송·Flow control·DMA는 확장 설계입니다. 이번 필수 범위에 몰래 추가하지 않습니다. Hardware Error flag 처리, 실제 Baud·RX/TX 파형·FIFO 서비스 지연은 Board 단계에서 확인합니다.

## 문서의 어느 부분이 판단의 근거인지 남깁니다.

| 질문 | 찾을 근거와 적용 |
| --- | --- |
| 어떤 MCU와 Pin인가? | D0 Addendum Table 1 p3~4: TC387QP Part·Package. D1 §2.2 p160: BGA292 Pin. Board Schematic과 대조. |
| Baud는 어떻게 만들어지는가? | U1 §36.3.3.1 p36-24~25: Clock divider. U2 p9-24~26: SCU_CCUCON2. S3 getFaFrequency: 선택 Clock 추적. |
| 8N1과 FIFO는 무엇을 보장하는가? | U1 §36.1 p36-2: 각각 16 Byte FIFO. §36.3.4 p36-28: Frame. Application line 경계는 별도 설계. |
| 초기화와 부분 I/O는? | S1 initSerialInterface L213~256, S2 initModuleConfig L353~405, S4 read L570~577 / write L615~644. |
| 문자열·변환의 의미는? | C1 N1570 §7.1.1, §7.21.6.5, §7.22.1.4. NUL, snprintf 길이, strtoul 허용 문법을 제품 규격과 비교. |

[D0 · TC38x Data Sheet Addendum](https://www.infineon.com/assets/row/public/documents/10/49/infineon-tc38x-addendum-datasheet-en.pdf)

[U1 · TC3xx User Manual Part 2, ASCLIN Chapter 36](https://www.infineon.com/assets/row/public/documents/10/44/infineon-aurix-tc3xx-part2-usermanual-en.pdf)

[S1 · TC387 TFT 공식 UART 초기화 Source](https://github.com/Infineon/AURIX_code_examples/blob/6759d2845feaac0107b785fe6bc57ed785c3f4fc/code_examples/ASCLIN_Shell_UART_1_KIT_TC387_TFT/ASCLIN_Shell_UART.c#L213)

[C1 · WG14 N1570 C11 Committee Draft](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf)

p는 문서의 인쇄 페이지입니다. 전체 Source URL과 다른 자료는 REFERENCES.md를 참조합니다. TC38x Appendix의 ASCLIN은 Chapter 34이므로 공통 Manual의 Chapter 36과 구분합니다.

> 공식 예제는 Clock·Pin·ISR·FIFO의 연결을 분석하는 근거입니다. 예제의 Shell Protocol과 이번 S,...,LF Protocol은 다릅니다. 예제를 복사하는 것만으로 본 과제의 요구가 충족되지는 않습니다.

## 설정 오류와 Parser 오류를 분리해 확인합니다.

> 여기부터는 자신의 문제 정의를 대조하는 해설입니다. 각 원인이 현상을 어떻게 만드는지 코드와 Test로 다시 확인합니다.

### RC01. 합의한 Baud가 설정에 반영되지 않았다

현상: uart_configure 실행 후 Host 관찰값은 baud=9600이다. 요구값은 115200이다. 원인은 uart_config.c에서 현재 설정이 합의한 통신 속도와 다르게 지정된 것이다. 문자열 Parser를 바꾸어 이 설정 차이를 해결할 수는 없다.

해결방안: Platform에 115200을 요청하고 UART-config의 baud 검사로 확인한다. Reference의 Baud만 9600으로 되돌린 M01 변이는 1개 Check에서 실패했다. 실제 Board에서는 ASCLIN 입력 Clock·분주와 상대 설정을 확인하고 Bit time을 측정해야 한다. Host 설정값만으로 실제 UART 파형이 정확하다고 주장하지 않는다.

### RC02. 수신 기능 요구가 RX enable에 반영되지 않았다

현상: 설정 관찰값 rx=0인 반면 요구는 TX·RX 모두 enabled이다. 원인은 수신 경로 enable 설정 누락이다. 송신이 된다는 사실만으로 수신 설정까지 정상이라고 판단할 수 없다. RX 활성화를 명시하고 UART-config의 rx_enabled 검사로 확인한다. M02 변이는 1개 Check에서 실패했다.

이 Host는 설정값만 기록하므로 rx=0이 실제 수신 Pin에서 어떤 파형 오류를 만드는지는 재현하지 않는다. 실제 TC387QP에서는 Pin MUX·Clock·RX 경로·Driver Buffer·ISR도 확인한다. 이 설정 Test와 Byte를 직접 주입하는 Parser Test를 독립적으로 실행해야 두 원인을 섞지 않는다.

> 잘못된 대안: Baud를 임의로 높여 처리량 문제를 숨기거나, 실제 Board 확인 없이 Pin이 고장 났다고 결론 내리는 것. 이번 설정의 정답은 제품 요구와 관찰 계약에서 결정됩니다.

## 숫자 범위와 전체 문법을 각각 입증합니다.

### RC03. 좁은 타입으로 변환하면 범위 검사도 끝난다고 보았다

현상: S,65536,1,1 / S,1,4096,1 / S,1,2,4처럼 의미 범위를 넘은 line을 수용한다. seq의 65536은 uint16_t로 보존되지 않는다. value=4096과 status=4는 C 타입에는 들어가지만 제품이 정한 범위에는 들어가지 않는다. Cast는 유효성 검사가 아니다.

해결방안: 각 필드의 상한을 곱셈·덧셈 전에 확인하고 검증된 값만 변환한다. Reference는 digit > maximum을 먼저 검사한 뒤 result > (maximum-digit)/10을 확인한다. 이 순서는 Unsigned 뺄셈의 Underflow를 피한다. 앞의 개념 페이지에서 설명한 몫·나머지 비교식도 같은 상한을 검사하는 방식이다.

Invalid range와 huge decimal Test로 경계를 확인한다. value 범위만 65535로 늘린 M03 변이는 2개 Check에서 실패했다. 값이 작아진 뒤 범위를 검사하거나 전체 필드에 uint16_t 최대값을 공통 적용하는 수정은 요구를 충족하지 않는다.

### RC04. 일부 숫자를 읽은 것을 전체 문법 성공으로 보았다

현상: 01·+1·공백 같은 비정규 숫자, 마지막 숫자 뒤의 X·CR·추가 필드를 수용한다. 원인은 숫자 변환의 느슨한 문법과 마지막 필드 이후 전체 소비를 확인하지 않는 것이다. Parse 입력은 길이가 정해진 Byte span이므로 NUL 뒤의 Byte도 숨기지 않는다.

해결방안: 숫자 문자·선행 0·구분자·필드 수를 확인하고 마지막 위치가 len과 같은지 검사한다. Invalid grammar와 NUL/non-ASCII/bounds Test를 대조한다. 선행 0 허용 변이는 6개, 뒤쪽 소비 검사 제거 변이는 20개 Check에서 실패했다. trim으로 CR·공백을 몰래 허용하면 LF-only Canonical 계약이 바뀐다.

> RC03은 값의 범위, RC04는 표현과 전체 구조의 문제입니다. 같은 Parse 함수 안에 있더라도 서로 다른 실패 입력과 검증 근거로 설명합니다.

## 반영 시점과 메시지 완료 시점을 지킵니다.

### RC05. 전체 성공 전에 호출자의 결과를 바꾸었다

현상: 이전 값이 {9,8,1}인데 S,99,8,x를 Parse하면 false를 반환하면서 seq가 99로 바뀐다. 원인은 앞의 필드를 읽자마자 out에 쓰는 처리 순서다. 반환값만 검사하면 실패 시 출력 훼손을 발견하지 못한다.

해결방안: 지역 후보에서 전체 필드와 끝 위치를 검사하고 성공할 때만 *out에 복사한다. RX도 정상 후보를 확인한 뒤 last와 have_last를 갱신한다. Invalid record output preservation Test에서 Sentinel 보존을 확인한다. 조기 출력 변경 M05 변이는 16개 Check에서 실패했다. false 직전에 out을 0으로 만드는 것도 이전 값 보존 요구에 어긋난다.

### RC06. Chunk 경계를 Application Frame 경계로 보았다

현상: S,1,23과 ,0 + LF로 나누면 정상 Frame이 유실되고, LF 없는 S,9,8,1은 미리 수용된다. 원인은 호출 시작마다 조립 상태를 초기화하고 호출 종료 때 Parse하는 두 동작이 Chunk와 Frame을 같은 단위로 가정하기 때문이다.

해결방안: used와 line을 호출 사이에 보관하고 LF에서만 검증한다. 무입력 호출도 부분 상태를 보존한다. RX-every-two-chunk-split, one-byte-chunks, incomplete Test로 정상 최대 Frame의 모든 분할점과 LF 이전 상태를 확인한다. Chunk 초기화만 다시 넣은 M06 변이는 75개 Check에서 실패했다.

> 가설 구별: 완성 line을 frame_parse에 직접 넣었을 때 정상인데 split feed에서만 실패하면, 숫자 계산보다 조립 상태·완료 조건을 먼저 조사합니다. 하지만 한 원인을 고쳤다고 다른 Parse 오류까지 사라졌다고 판단하지 않습니다.

## 같은 입력의 나머지와 다음 줄의 경계를 보존합니다.

### RC07. 첫 LF에서 전체 입력 처리를 끝내었다

현상: 한 Chunk에 정상 Frame 둘을 넣어도 첫 Frame만 처리하거나, 첫 줄이 오류이면 뒤 정상 줄까지 사라진다. 원인은 첫 LF 처리 직후 함수 전체에서 return하는 것이다. LF는 현재 line의 끝이며 입력 배열 전체의 끝이 아니다.

해결방안: LF마다 현재 line만 판정·초기화하고 다음 Byte를 계속 처리한다. accepted는 이번 호출의 정상 Frame 수를 누적하며 last는 마지막 정상 Record를 보관한다. RX-joined-valid-and-invalid Test로 확인한다. 첫 LF return M07 변이는 158개 Check에서 실패했다. 마지막 값 하나만 보관하는 정책과 여러 Frame을 무시하는 오류는 다르다.

### RC08. 용량 초과 후 같은 줄의 뒷부분을 새 줄로 보았다

현상: 너무 긴 줄의 뒤에 붙은 S,1,2,3 조각을 정상 Frame으로 수용한다. 원인은 32번째 일반 Byte에서 길이만 0으로 되돌려 같은 line의 suffix를 다시 모으는 것이다. Buffer 범위를 넘지 않더라도 메시지 경계를 잘못 복구할 수 있다.

해결방안: 용량 초과면 discarding 상태로 전환하고 LF까지 버린다. LF 후에만 새 line을 모은다. 이번 discarding은 용량 초과 전용이며 문법 검사는 LF 시점에 한다. RX-overlong-through-LF-recovery와 line-capacity-state, Overflow 입력의 모든 분할점에서 확인한다. M08 변이는 50개 Check에서 실패했다.

> Buffer를 크게 만드는 것만으로 RC08을 해결할 수 없습니다. 더 긴 입력에서 같은 경계 손실이 생깁니다. 복구 기준을 LF로 정한 이유와 오류 뒤 정상 줄을 수용하는 결과를 함께 설명합니다.

## 미완료 Frame과 실제 수락량을 추적합니다.

### RC09. Busy 상태의 Pending Frame을 덮어썼다

현상: A가 아직 전송 중인데 B를 queue하면 A의 남은 내용이 바뀐다. 원인은 Busy 확인 없이 같은 Pending Buffer에 새 Record를 Encode하는 것이다. 32 Byte Buffer라는 저장 크기가 여러 Frame 수용을 허용한다는 뜻은 아니다.

해결방안: Encode 전에 Busy를 거부하고 Byte·length·offset 모두 보존한다. TX-busy-whole-frame-preservation Test로 확인한다. M09 변이는 41개 Check에서 실패했다. 이미 Encode한 뒤 false만 반환하면 데이터가 훼손된 뒤이므로 충분하지 않다. 상위 호출자는 거부된 Record의 재시도·폐기 정책을 별도로 기록해야 한다.

### RC10. write 호출을 전체 수락 완료로 간주했다

현상: 최대 Frame 15 Byte에 수락 한도 3/0/2/32를 적용하고 pump를 4번 호출하면 원본은 S,6의 3 Byte만 남기고 Idle이 된다. 원인은 실제 반환 수를 무시하고 첫 write 직후 Pending을 비우는 것이다. 0 Byte 수락도 같은 원인으로 데이터가 사라진다.

해결방안: 한 pump에서 write 한 번, 반환 수만 offset 증가, 완료 전까지 상태를 보존한다. Reference 관찰 길이는 3→3→5→15이고 write는 4회다. TX-partial-zero-and-order를 확인한다. M10 변이는 51개 Check에서 실패했다. 전체 Frame을 처음부터 재송신하면 prefix가 중복되고, 0일 때 무한 반복하면 비차단 계약을 깨뜨린다.

> 수락 완료는 Driver로 Byte 소유권이 넘어간 상태입니다. 실제 Wire 완료·상대 처리 완료와 다릅니다. 실제 iLLD의 bool·count·timeout 계약을 Host Platform 계약에 연결하는 별도 Adapter가 필요합니다.

## Test 결과를 근거와 학습 목표로 마무리합니다.

| 2026-10-24 Host 실행 | 결과 |
| --- | --- |
| Starter / 고정 Test | 584 Check 중 252 실패. Build 성공, Test 실행 파일 Exit 1. |
| Reference / 같은 고정 Test | 584 Check 모두 통과. Test Exit 0. |
| Reference / 추가 Test 포함 | 622 Check 모두 통과. ASan/UBSan Diagnostic 없음. |
| 한 원인을 다시 넣은 변이 | 11개 모두 Build 성공 뒤 Test 실패. RC04를 두 변이로 나누어 검증. |

252개 실패는 252개의 독립 원인이 아닙니다. 모든 실패 Check를 RC01~10에 연결한 대응표와 실제 명령·Source Hash를 로컬 검증 기록에 보관했습니다. 모든 원인을 설명하되, 같은 원인이 여러 분할점에서 반복 검출되는 사실과 원인 간 상호작용을 구분합니다.

### 예상 후속 질문

LF가 있는데 왜 len도 필요한가? LF는 메시지의 끝이고 len은 지금 접근할 수 있는 입력 범위다. 한 Chunk에 LF가 없거나 여러 개 있을 수 있으므로 둘 다 필요하다.

seq가 65535에서 0이 되면 오류인가? 이번 규격은 표현과 범위만 검사한다. 순번 Wrap·중복·순서 역전을 거부하는 업무 정책은 별도 요구다.

모든 Test가 통과하면 Board에서도 정상인가? Host의 Byte·상태 계약은 확인했지만 실제 ASCLIN Clock·Pin·ISR·전압·파형·Timing은 별도 검증해야 한다. Sanitizer도 Hardware 동작의 증명은 아니다.

> 완료 후 확인: UART 설정의 근거를 찾았다. 기록 규격과 Byte Stream을 구분했다. 잘림·오류·혼잡에서 상태를 보존했다. 모든 근본 원인을 현상–원인–해결방안으로 설명하고 Test·처리량 계산과 연결했다.
