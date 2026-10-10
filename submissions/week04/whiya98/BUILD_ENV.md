# 4주차 Build 환경 기록

빈 칸은 본인이 확인한 값으로 채웁니다. Source에서 읽은 사실과 실제 실행 결과를 구분합니다.

| 항목 | 확인한 내용 |
| --- | --- |
| GitHub 아이디 / 작업 Branch | |
| 과제 기준 upstream Commit | |
| OS / CPU | |
| C Compiler 이름 / version | |
| GNU Make version | |
| 실제 CC / CFLAGS / ASFLAGS | |
| Host test 실행 위치 / 명령 / Exit code | |
| 단계별 Build 실행 위치 / 명령 / Exit code | |
| 실행 File 형식 | |
| 실행 Log | logs/test.log, logs/stages.log |

## 공개 STM32 예제의 기준

- MCU / Board:
- Datasheet 번호 / Revision:
- Source URL / Version / Commit:
- 분석한 Project 경로:
- Target / Include / Define / Linker 설정:
- HAL / CMSIS / BSP 의존성:
- 실제 ARM Compiler / IDE: 미설치라면 '미설치'
- 실제 ARM Build / Board 실행: 실행하지 않았다면 '미실행'

## 동료의 재현 절차

1. 제출 PR의 Branch 또는 Commit:
2. 진입할 제출 폴더:
3. 필요한 도구:
4. Clean Build와 Test 명령:
5. 기대 결과와 확인할 Log:

Host test 통과는 실제 MCU에서 Clock 주파수·LED 주기를 측정한 결과가 아닙니다. Binary는 제출하지 않고 다시 만드는 데 필요한 Source와 설정·명령을 기록합니다.
