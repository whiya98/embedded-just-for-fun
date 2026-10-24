# [07회차] YOUR_GITHUB_ID TC387QP UART 기록·Parsing 개선

## 현상과 개선 결과

재현 입력과 전달 순서, 원본의 실제 결과, 변경 후 결과를 요약합니다. 입력 Byte와 Chunk 경계를 구분해 적습니다.

## 현상–원인–해결방안

| 문제 ID | 현상: 조건·기대·실제 | 원인 가설과 구별 Test | 확정 원인: 함수·깨진 규칙 | 해결방안과 선택 이유 | 수정 전후·추가 Test |
| --- | --- | --- | --- | --- | --- |
| P01 | 직접 작성 | | | | |

`PROBLEM_DEFINITION.md`의 전체 기록과 연결합니다. 실패 Check 개수를 근본 원인 개수로 쓰지 않습니다. 동일 원인의 중복 증상과 독립된 원인을 구분하고, 계약의 경계를 빠짐없이 검토합니다.

## 기술 근거와 설계 판단

| 공식 자료 ID / Version | 절·페이지·함수 | 확인한 사실 | 이번 요구에 적용한 판단 | 검증 근거 |
| --- | --- | --- | --- | --- |
| 직접 작성 | | | | |

- ASCLIN Clock·Baud·8N1·TX/RX와 Host 설정 요청의 관계
- Chunk / Application Frame / C String의 경계와 길이
- Parsing 실패 시 상태 보존, 긴 줄 이후 복구
- 부분 write와 0 진행, Busy 거부, Driver 수락과 Wire 완료의 차이
- 100 Hz / 1000 Hz 전송량과 혼잡 정책

## 실제 검증

| 실행 | Exit code / 결과 | Log |
| --- | --- | --- |
| 원본 `make test` | 직접 실행 결과 | `logs/before-test.log` |
| 원본 `make demo` | 직접 실행 결과 | `logs/before-demo.log` |
| 개선 후 `make test` | 직접 실행 결과 | `logs/test.log` |
| 개선 후 `make demo` | 직접 실행 결과 | `logs/demo.log` |
| 추가 Test 1 | 가설·입력·예상·실제 | 함수명 |
| 추가 Test 2 | 가설·입력·예상·실제 | 함수명 |

실제 환경은 `BUILD_ENV.md`에 적습니다. ASan/UBSan을 사용했다면 명령과 결과도 남깁니다. Host에서 입증한 범위와 실제 Board에서 확인해야 할 사항을 구분합니다.

## 제출 확인

- [ ] 자기 `submissions/week07/YOUR_GITHUB_ID/`만 변경했습니다.
- [ ] 원본·제공 API·Host·고정 Test를 약화하지 않았습니다.
- [ ] 모든 근본 원인을 재현·코드·근거·수정·Test에 연결했습니다.
- [ ] 서로 다른 경계의 추가 Test가 최소 2개입니다.
- [ ] PDF·ZIP·Build 산출물·SDK 전체·인증 정보는 포함하지 않았습니다.
- [ ] Base는 `zinee-u/embedded-just-for-fun:main`입니다.
