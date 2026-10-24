# 원본 실행 관찰 자료

2026-10-24 macOS 26.3.1 arm64 / Apple clang 21.0.0 / GNU Make 3.81에서 제공 Starter를 실행한 출력입니다. 기본 옵션은 `-std=c11 -Wall -Wextra -Wpedantic -Werror -O2`이며 실제 Makefile의 추가 옵션도 확인합니다. 각 실행 파일의 stdout을 아래 파일에 보관했습니다. 이 파일은 원인 해설이 아닌 재현 결과입니다.

- `original-test.log`: 고정 검사 584개 중 252개 실패, 실행 파일 Exit 1. `make test`의 Make Exit는 2입니다.
- `original-demo.log`: 설정값, 분할·합쳐진 RX 입력, 부분 TX 수용 결과. Demo Exit 0은 요구 충족을 의미하지 않습니다.
- 원본을 직접 실행해 결과를 비교하고, 자신의 OS·Compiler·명령·Exit code를 별도로 기록합니다.
- Host 결과이며 실제 TC387QP Baud·Pin·Hardware FIFO·Board 실행 결과가 아닙니다.
