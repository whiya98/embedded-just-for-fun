# Embedded Just for Fun

스누피와 함께 C 코드를 읽고, 문제의 원인을 찾아 고치는 주차별 과제 저장소입니다. 수정 전 코드와 문제를 공개하고, 개선한 코드·설명·실행 결과를 Pull Request로 제출합니다.

## 이번 과제

| 주차 | 주제 | 문제 | 수정 전 코드 |
| --- | --- | --- | --- |
| 2주차 | 자료형과 배열, 메모리와 포인터 | [와이퍼 센서값 변환](weeks/week02/README.md) | [decode_rain.c](weeks/week02/starter/decode_rain.c) |
| 3주차 | 비트 연산, 레지스터 제어, volatile | [탐사 장치 레지스터 제어](weeks/week03/README.md) | [register_control.c](weeks/week03/starter/register_control.c) |
| 4주차 | MCU 구조, Build, 초기화, Git 관리 | [MCU 초기화 기능 구현](weeks/week04/README.md) | [네 Module과 Host test](weeks/week04/starter/) |
| 05회차 | TC387QP GPIO, STM Timer, Polling, 주기 수집 | [주기 수집 기능 개선](weeks/week05/README.md) | [세 Module과 Host test](weeks/week05/starter/) |
| 06회차 | TC387QP Interrupt, ISR, 공유 데이터 처리 | [Interrupt 수집 개선](weeks/week06/README.md) | [세 Module과 Host Test](weeks/week06/starter/) |
| 07회차 | TC387QP UART 기록, Frame, Parsing, 부분 송신 | [UART 기록·Parsing 개선](weeks/week07/README.md) | [네 Module과 Host Test](weeks/week07/starter/) |

`week02`, `week03`, `week04`, `week05`, `week06`, `week07`은 수업 회차입니다. 달력의 월별 주차나 제출 마감일을 뜻하지 않습니다.

**4주차를 시작한다면 [4주차 전용 제출 안내](weeks/week04/SUBMISSION.md)를 따라 하세요.** MCU 요구값을 네 `.c` Module에 구현하고, Host test와 Build 단계 확인을 마친 뒤 실제 PR을 제출합니다.

**05회차는 [한국어 해설 코드](weeks/week05/reference/README.md)를 함께 공개합니다.** 해설과 근거를 읽고 원본 Starter를 개선한 뒤, Timing 분석과 직접 작성한 추가 Test·실행 결과를 기존 방식의 PR로 제출하세요. 자세한 순서는 [05회차 제출 안내](weeks/week05/SUBMISSION.md)를 따릅니다.

**06회차는 [풀이 접근법](weeks/week06/APPROACH.md)을 함께 제공합니다.** 학습 목표·공식 근거·11개 근본 원인을 참고해 세 C Module을 직접 개선하고, 추가 Test와 실행 결과를 [06회차 제출 안내](weeks/week06/SUBMISSION.md)에 따라 PR로 제출하세요.

**07회차는 [원본 실행 Log](weeks/week07/baseline/README.md)와 [풀이 접근법](weeks/week07/APPROACH.md)을 함께 제공합니다.** UART 기록의 현상·원인·해결방안을 근거로 정리하고 네 C Module을 직접 개선합니다. 분할·합쳐진 RX 입력과 부분 TX 수용을 검증한 뒤, 추가 Test와 실행 결과를 [07회차 제출 안내](weeks/week07/SUBMISSION.md)에 따라 PR로 제출하세요.

## 과제 풀기

1. 주차별 문제를 읽고 이 저장소를 **Fork**합니다.
2. 자신의 Fork에서 과제용 브랜치를 만듭니다.
3. 해당 주차의 원본을 `submissions/weekNN/<자신의 GitHub 아이디>/`로 복사합니다. `weekNN`은 `week02`, `week03`처럼 실제 주차로 바꿉니다.
4. 코드를 고치고 테스트와 원인 분석을 작성합니다.
5. 이 저장소의 `main`을 대상으로 PR을 보냅니다.

처음 GitHub로 제출한다면 [PR 제출 안내](CONTRIBUTING.md)를 순서대로 따라 하세요. PR 제목은 `[week02] <GitHub 아이디> 와이퍼 센서값 변환 개선`처럼 작성합니다.

## 폴더 구조

```text
weeks/
  week02/
    README.md                 # 문제와 요구 조건
    starter/decode_rain.c     # 개선 전 함수 원본
    cases.csv                # 요구 조건 확인용 입력과 예상 결과
  week03/
    README.md                 # 레지스터 제어 문제와 선언 문항
    starter/register_control.c # 개선 전 네 함수
    cases.csv                # 예상값과 실행값을 직접 기록할 일곱 입력
  week04/
    README.md                 # MCU 초기화 제품 요구사항
    SUBMISSION.md             # Fork부터 PR까지 4주차 전용 절차
    PR_BODY.md                # 4주차 PR 작성양식
    REFERENCES.md             # Datasheet와 고정 공개 예제 링크
    starter/                  # 네 .c Module, API, Host 대역, tests, Makefile
  week05/
    README.md                 # TC387QP GPIO·STM·Polling 요구사항
    SUBMISSION.md             # 해설 학습부터 기존 방식의 PR 제출까지
    PR_BODY.md                # 근거·이해한 내용·검증을 적는 양식
    REFERENCES.md             # Datasheet·Board·User Manual·공식 예제
    starter/                  # 개선 전 세 Module과 Host test
    reference/                # 05회차에 한해 공개하는 한국어 해설 코드
  week06/
    README.md                 # TC387QP Interrupt·ISR·공유 Mailbox 요구사항
    SUBMISSION.md             # 자기 제출 폴더에서 개선 후 PR 제출
    PR_BODY.md                # 모든 원인·공식 근거·Test 기록 양식
    REFERENCES.md             # Datasheet·STM·Interrupt·iLLD 공식 근거
    APPROACH.md               # 공개 풀이 접근법과 모든 근본 원인
    starter/                  # 개선 전 세 Module과 결정적 Host Test
  week07/
    README.md                 # TC387QP UART 기록·Parsing 개선 요구사항
    SUBMISSION.md             # 원본 관찰부터 개선·검증·PR 제출까지
    PR_BODY.md                # 현상·원인·해결방안·검증 기록 양식
    REFERENCES.md             # ASCLIN·UART·iLLD 공식 근거
    APPROACH.md               # 공개 풀이 접근법과 근본 원인 분석
    baseline/                 # 원본 Host Test와 Demo 실행 Log
    starter/                  # 개선 전 네 Module, Host Test, 분석 양식
submissions/
  week02/<github-id>/         # 학생별 PR로 추가
    decode_rain.c            # 자신의 개선 코드
    tests.c                  # 자신의 테스트
    README.md                # 근본 원인, 설계, 실행 결과
  week03/<github-id>/
    register_control.c       # 자신의 개선 코드
    tests.c                  # 자신의 테스트
    README.md                # 원인, 결과, 한정자 선언 설명
  week04/<github-id>/
    src/                     # 구현한 Memory·Clock·LED·Application
    tests/                   # 제공 검증과 본인 추가 test
    logs/                    # 실제 Host test·Build 단계 실행 결과
    README.md                # 원인·변경·검증 설명
    BUILD_ENV.md             # 도구·Source 기준·실행 환경
    TRACE.md                 # Datasheet·Build·Startup·Module 근거
  week05/<github-id>/
    src/                     # 직접 개선한 GPIO·Timer·Collector
    tests/                   # 제공 검증과 직접 추가한 Test
    logs/                    # 본인이 실행한 Test와 Demo 결과
    README.md                # 근본 원인·수정 원리·근거
    BUILD_ENV.md             # 실제 실행 환경
    TIMING.md                # Tick 계산·호출표·지연 분석
  week06/<github-id>/
    src/                     # 직접 개선한 Mailbox·ISR·Collector
    tests/                   # 고정 Test와 직접 추가한 경계 Test
    logs/                    # 실제 수정 전·후 Test와 Demo 결과
    README.md                # 모든 근본 원인과 수정 근거
    BUILD_ENV.md             # 실제 실행 환경
    CONCURRENCY.md           # 흐름도·경쟁 순서·IRQ 복구·지연 분석
  week07/<github-id>/
    src/                     # 직접 개선한 UART 설정·Codec·RX·TX
    tests/                   # 고정 Test와 직접 추가한 경계 Test
    logs/                    # 실제 수정 전·후 Test와 Demo 결과
    README.md                # 최종 변경 요약
    BUILD_ENV.md             # 실제 실행 환경과 검증 기록
    PROBLEM_DEFINITION.md    # 현상·가설·근본 원인·해결방안·검증
    PROTOCOL.md              # Byte 규격·상태 전이·처리량·공식 근거
templates/
  week/README.md              # 새 주차 문제 작성 양식
  submission/README.md        # 학생 답변 작성 양식
docs/
  WEEKLY_MAINTENANCE.md       # 매주 과제 추가와 PR 운영
.github/
  pull_request_template.md    # PR 작성 시 표시되는 양식
```

## 운영 원칙

- `weeks`에는 항상 개선 전 코드와 문제를 보존합니다.
- 05회차만 `reference`의 한국어 해설 코드를 함께 제공합니다. 참고 구현을 읽고 스스로 근거를 설명하며 검증하는 학습 방식입니다. 다른 회차의 비공개 해설은 포함하지 않습니다.
- 06회차는 풀이 접근법 Markdown 문서를 공개합니다. 내용을 참고해 직접 개선 코드를 작성하고 추가 Test와 실제 실행 결과를 제출합니다.
- 07회차는 원본 실행 Log와 풀이 접근법 Markdown 문서를 공개합니다. 완성 참고 구현과 내부 검증 자료는 포함하지 않으며, 관찰한 현상과 원인 가설을 구분해 직접 개선하고 검증합니다.
- PDF와 ZIP은 로컬에서 관리하며 이 저장소에는 올리지 않습니다.
- 풀이 PR은 자기 `submissions` 폴더에 제출합니다. 원본이나 다른 사람의 답안을 덮어쓰지 않습니다.
- 증상 한 개만 고치는 것으로 끝내지 않고, 문제의 근본 원인을 모두 찾아 설명합니다.
- 정답 구현을 한 가지로 고정하지 않습니다. 요구 조건을 만족하는지, 선택한 방식의 근거와 단점을 설명하는지 함께 봅니다.
- API 변경 범위와 필수 도구·테스트는 주차별 문제를 따릅니다. 4주차는 제공된 API와 검증 기준을 유지합니다.
- 실행 결과에는 실제로 실행한 환경과 결과를 적습니다. 예상값을 실행값으로 표시하지 않습니다.
- 병합된 제출물과 공개 PR은 다른 사람도 볼 수 있습니다. 독립 풀이가 목적이라면 자신의 첫 답안을 만든 뒤 다른 PR을 참고하세요.

이 저장소는 학습용입니다. `starter`의 코드는 수정할 결함을 포함한 원본입니다. 전체 코드와 입력 조건을 검토하기 전에 그 결과를 제어 명령에 사용하지 마세요.

출제자는 [매주 관리 안내](docs/WEEKLY_MAINTENANCE.md)를 참고합니다.
