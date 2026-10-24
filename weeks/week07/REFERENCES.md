# 07회차 공식 자료와 근거 기록

조회 기준일: **2026-10-24 (Asia/Seoul)**. `p`는 문서 하단의 **인쇄 페이지**입니다. PDF Viewer의 페이지 번호와 구분하세요. 문서가 보장하는 사실, 이번 요청서가 정한 정책, 직접 실행한 Test 결과를 따로 기록합니다.

## 대상과 자료의 역할

대상은 **TC387QP / TC38x 계열**이며 비교 기준 Part는 **SAK-TC387QP-160F300S AE, PG-LFBGA-292**입니다. 이번 Host 과제는 실제 Board에 바로 Flash하는 전체 Firmware가 아닙니다.

| ID | 공식 자료 | 확인할 위치와 질문 |
| --- | --- | --- |
| D0 | [TC38x Data Sheet Addendum](https://www.infineon.com/assets/row/public/documents/10/49/infineon-tc38x-addendum-datasheet-en.pdf), v1.9, 2025-12 | Table 1 p3~4. 정확한 Part·Package·CPU 최대 Clock·ASCLIN 구성을 찾습니다. CPU 최대 300 MHz가 UART Baud rate나 실제 ASCLIN Clock을 뜻하나요? |
| D1 | [TC38x Datasheet](https://www.infineon.com/assets/row/public/documents/10/49/infineon-tc38x-datasheet-en.pdf), V1.2, 2021-03 | §2.2 p160부터 BGA292 Pin 정의, §3.4 p291 Operating Conditions, §3.5 p294부터 Pad 조건. Package와 Board 배선을 함께 확인해야 하는 이유를 설명합니다. |
| U1 | [TC3xx User Manual Part 2](https://www.infineon.com/assets/row/public/documents/10/44/infineon-aurix-tc3xx-part2-usermanual-en.pdf), V2.0.0, 2021-02 | §36.1 p36-2: FIFO와 ASC 기능. §36.3.3.1 p36-24~25: Baud rate 생성. §36.3.4 p36-28: Data frame. §36.3.11 p36-45~46: TX·RX·Error 요청. |
| U2 | [TC38x User Manual Appendix](https://www.infineon.com/assets/row/public/documents/10/44/infineon-aurix-tc38x-usermanual-en.pdf), V2.0.0, 2021-02 | p9-24~26의 SCU_CCUCON2, §34.4 Table 337 p34-49~50의 ASCLIN0 연결. 공통 Manual의 ASCLIN은 Chapter 36, 이 Appendix에서는 Chapter 34입니다. |
| U3 | [ASCLIN 공식 개요](https://documentation.infineon.com/aurixtc3xx/docs/dma1710341265971) | Feature list / Functional description. Shift Engine과 FIFO를 다루는 User Interface의 역할을 나누어 읽습니다. |
| C1 | [WG14 N1570: C11 Committee Draft](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf), 2011-04-12 | §7.1.1¶1 p180의 C String, §7.21.6.5 p325의 `snprintf`, §7.22.1.4 p344~345의 `strtoul`. Buffer 크기, 실제 송신 길이, 변환 성공의 조건을 구분합니다. |

Datasheet는 제품과 전기적 조건을, User Manual은 Register와 동작을, iLLD Source는 API의 실제 계약을 확인하는 자료입니다. `S,<seq>,<value>,<status>\n` 형식과 값 범위·선행 0 제한·혼잡 시 정책은 **이번 요청서의 Protocol**이며 MCU 자체의 요구가 아닙니다.

## 공식 예제를 함수까지 추적합니다

기준 Repository는 `Infineon/AURIX_code_examples`, 고정 Commit은 **6759d2845feaac0107b785fe6bc57ed785c3f4fc**입니다. 예제 경로는 `code_examples/ASCLIN_Shell_UART_1_KIT_TC387_TFT`입니다. 아래 Source를 직접 확인했습니다.

| ID | 고정 Source | 읽을 위치·질문 |
| --- | --- | --- |
| S1 | [ASCLIN_Shell_UART.c](https://github.com/Infineon/AURIX_code_examples/blob/6759d2845feaac0107b785fe6bc57ed785c3f4fc/code_examples/ASCLIN_Shell_UART_1_KIT_TC387_TFT/ASCLIN_Shell_UART.c#L213) | L213~256 `initSerialInterface`: 기본 Config → Baud rate·Bit timing → Interrupt → Pin → Software FIFO → Module 초기화. L110~129의 세 ISR도 연결합니다. |
| S2 | [IfxAsclin_Asc.c: 기본 설정](https://github.com/Infineon/AURIX_code_examples/blob/6759d2845feaac0107b785fe6bc57ed785c3f4fc/code_examples/ASCLIN_Shell_UART_1_KIT_TC387_TFT/Libraries/iLLD/TC38A/Tricore/Asclin/Asc/IfxAsclin_Asc.c#L353) | L353~405 `IfxAsclin_Asc_initModuleConfig`. `clockSource`, `frame`, `fifo`, Interrupt priority와 Buffer 기본값을 실제 초기화값과 비교합니다. |
| S3 | [IfxAsclin.c: Clock 조회](https://github.com/Infineon/AURIX_code_examples/blob/6759d2845feaac0107b785fe6bc57ed785c3f4fc/code_examples/ASCLIN_Shell_UART_1_KIT_TC387_TFT/Libraries/iLLD/TC38A/Tricore/Asclin/Std/IfxAsclin.c#L96) | L96~118 `IfxAsclin_getFaFrequency`. 선택한 Clock source에서 주파수를 구하는 경로를 추적합니다. CPU 최대 Clock을 그대로 넣으면 안 되는 근거를 찾습니다. |
| S4 | [IfxAsclin_Asc.c: read / write](https://github.com/Infineon/AURIX_code_examples/blob/6759d2845feaac0107b785fe6bc57ed785c3f4fc/code_examples/ASCLIN_Shell_UART_1_KIT_TC387_TFT/Libraries/iLLD/TC38A/Tricore/Asclin/Asc/IfxAsclin_Asc.c#L570) | L570~577 `read`, L615~644 `write`. `count` 입력·출력과 `boolean`의 의미를 비교합니다. `FALSE`인데 일부 Byte가 처리된 상황을 표현할 수 있나요? |
| S5 | [IfxAsclin_Asc.h: API 계약](https://github.com/Infineon/AURIX_code_examples/blob/6759d2845feaac0107b785fe6bc57ed785c3f4fc/code_examples/ASCLIN_Shell_UART_1_KIT_TC387_TFT/Libraries/iLLD/TC38A/Tricore/Asclin/Asc/IfxAsclin_Asc.h#L527) | L527~557의 `count`, 전체 처리 여부, System Timer Tick 단위 timeout. `write` 성공과 마지막 Stop bit 전송 완료가 같은 시점인지 확인합니다. |
| S6 | [Ifx_Fifo.c: write](https://github.com/Infineon/AURIX_code_examples/blob/6759d2845feaac0107b785fe6bc57ed785c3f4fc/code_examples/ASCLIN_Shell_UART_1_KIT_TC387_TFT/Libraries/iLLD/TC38A/Tricore/_Lib/DataHandling/Ifx_Fifo.c#L366) | L366~414. 공간이 부족할 때 대기하는 지점, Deadline 확인, 미처리 개수 반환을 확인합니다. 무한 timeout은 Main의 다른 작업에 어떤 영향을 줄까요? |
| S7 | [IfxAsclin_Asc.c: Error / RX ISR](https://github.com/Infineon/AURIX_code_examples/blob/6759d2845feaac0107b785fe6bc57ed785c3f4fc/code_examples/ASCLIN_Shell_UART_1_KIT_TC387_TFT/Libraries/iLLD/TC38A/Tricore/Asclin/Asc/IfxAsclin_Asc.c#L439) | L439~473의 Hardware Error flag, L476~515의 Hardware FIFO → Software FIFO 복사와 `rxSwFifoOverflow`. 어느 단계에서 Byte가 사라질 수 있나요? |

S1은 ASCLIN0에서 RX=P14.1, TX=P14.0을 선택한 **해당 Board 예제**입니다. TC387QP의 모든 Board가 같은 Connector로 연결된다고 일반화하지 않습니다. 실제 이식에서는 D1의 Package Pin, U2의 연결 가능성, Board Schematic, iLLD Pin mapping을 함께 확인합니다.

## 학습 목표와 근거 연결

| 학습 목표 | 찾을 근거 | 남길 검증 |
| --- | --- | --- |
| UART 설정과 송수신 흐름을 설명한다 | D0~D1, U1~U2, S1~S3 | 115200·8N1의 의미, Clock 경로, 초기화와 TX/RX/Error ISR의 역할. 실제 Baud 오차·전압·파형은 측정 여부를 구분합니다. |
| 기록 규격과 Streaming Parser를 만든다 | U3의 Byte 전달 역할, C1, 요청서의 Protocol | 동일 메시지의 모든 분할 위치, 여러 줄 병합, 미완성 꼬리, 경계값·잘못된 입력을 시험합니다. 읽기 호출 경계가 메시지 경계라는 가정을 검증합니다. |
| 잘림·오류·기록량의 영향을 분석한다 | S4~S7, U1의 Error 항목, 요청서의 송신율 | 부분 송신·0 Byte 처리·혼잡·복구 후 다음 정상 메시지, 최대 길이의 Wire time과 생성률을 기록합니다. |
| 현상–원인–해결방안을 스스로 정의한다 | 재현 Log → Source 계약 → 공식 근거 → 수정 후 Test | 관찰값과 추정을 분리하고, 후보 원인을 배제한 근거와 수정 후 같은 입력의 결과를 연결합니다. |

Hardware의 Framing error, Parity error, FIFO overflow와 Application의 필드 누락·숫자 범위 오류는 서로 다른 층의 문제입니다. 8N1은 Parity를 사용하지 않으므로 Parity 검출이 데이터 무결성을 보장한다고 쓰지 않습니다. Protocol 형식 검사가 통신 중 모든 Bit 손상을 검출하는 것도 아닙니다.

## 근거 기록 양식

| 관찰한 현상·재현 입력 | 원인 가설 | 문서 ID·Version·절·함수 | 확인한 사실·배제한 가설 | 해결방안·적용 조건 | 검증 방법·실제 결과 |
| --- | --- | --- | --- | --- | --- |
| 직접 작성 | 아직 입증되지 않은 설명 | 정확한 위치 | 관찰값과 근거를 연결 | 동작을 바꿀 원칙 | 수정 전·후를 같은 입력으로 비교 |

Host Test는 정해진 Byte 배열과 API 반환 계약에 대한 Software 검증입니다. 실제 ASCLIN Register, Board 배선, Interrupt latency, Baud rate 오차를 검증한 결과로 바꾸어 적지 않습니다.
