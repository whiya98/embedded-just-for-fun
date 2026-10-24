# 07회차 Build·검증 환경

- 작성자 / GitHub ID:
- OS / Version:
- C Compiler / Version (`cc --version`):
- Make Version (`make --version`):
- 실제 명령 / Flag:
- 검증한 Commit:

| 실행 | 변경 전 Exit / 검사·실패 수 | 변경 후 Exit / 검사·실패 수 | Log 경로 |
| --- | --- | --- | --- |
| `make` | | | |
| `./build/mission-tests` | | | |
| `./build/mission-demo` | | | |

Make의 실패 Exit와 Test 실행 파일의 Exit는 다를 수 있습니다. Test 실행 파일은 요구사항 불일치가 있으면 1을 반환합니다. `logs/test.log`처럼 Text Log는 제출할 수 있고 `build/`의 Binary는 제출하지 않습니다.

| 추가 Test | 입력·실행 순서 | 예상 결과 | 실제 결과 | 이 Test가 필요한 이유 |
| --- | --- | --- | --- | --- |
| 1 | | | | |
| 2 | | | | |

- 고정 Test 외에 내가 확인한 경계:
- Host에서 확인한 범위:
- 실제 Board 실행 여부 / 측정 장비 / 측정 결과:
- 미검증 사항:

Windows 10에서는 WSL 또는 MSYS2 등 C11 Compiler와 Make가 있는 환경에서 실행합니다. 제공 검증은 macOS의 `cc`로 수행했으며 Windows/Linux 실행을 수행한 것으로 간주하지 않습니다. Native Windows 실행 파일 이름이 필요하면 `make EXEEXT=.exe`를 사용하고 해당 환경에서 다시 확인합니다.
