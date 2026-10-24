# 07회차 과제를 Pull Request로 제출하기

[과제](README.md)를 읽고 자신의 Fork에서 Starter를 개선해 제출합니다. [문제 풀이 접근법](APPROACH.md)은 자신의 분석을 작성한 뒤 대조할 수 있는 참고 자료입니다. 명령은 macOS/Linux Terminal 또는 Git Bash 문법이며 Windows에는 C Compiler와 GNU Make 환경을 별도로 준비해야 합니다.

## 1. Fork와 Clone

[원본 저장소](https://github.com/zinee-u/embedded-just-for-fun)를 자신의 GitHub 계정으로 Fork합니다. 기존 Clone이 있으면 다시 만들지 않습니다.

```sh
git clone https://github.com/YOUR_GITHUB_ID/embedded-just-for-fun.git
cd embedded-just-for-fun
git remote add upstream https://github.com/zinee-u/embedded-just-for-fun.git
git remote -v
git config --local user.name "YOUR_GITHUB_ID"
git config --local user.email "YOUR_COMMIT_EMAIL"
```

이미 `upstream`이 있으면 추가하지 말고 URL만 확인합니다. `origin`은 자신의 Fork여야 합니다.

## 2. 자신의 Branch와 제출 폴더

기존 작업을 먼저 보관하고 시작합니다. 아래 복사 명령은 새 풀이를 시작할 때만 실행하며 기존 풀이를 덮어쓰지 않습니다.

```sh
git status --short
git fetch upstream
git switch -c solve/week07-YOUR_GITHUB_ID upstream/main
mkdir -p submissions/week07/YOUR_GITHUB_ID
cp -R weeks/week07/starter/. submissions/week07/YOUR_GITHUB_ID/
cd submissions/week07/YOUR_GITHUB_ID
```

제출 폴더에서 `git init`을 다시 실행하지 않습니다.

## 3. 원본의 현상부터 기록

```sh
cc --version
make --version
mkdir -p logs
make clean
make test > logs/before-test.log 2>&1
before_status=$?
printf '\nmake test exit code: %s\n' "$before_status" >> logs/before-test.log
cat logs/before-test.log
make demo > logs/before-demo.log 2>&1
demo_status=$?
printf '\nmake demo exit code: %s\n' "$demo_status" >> logs/before-demo.log
```

원본의 고정 Test는 의도된 결함을 검출하여 실패합니다. 실패를 지우거나 기대값을 원본 동작으로 낮추지 않습니다. Demo의 Exit 0도 요구사항 충족을 의미하지 않으므로 출력값을 비교합니다. `PROBLEM_DEFINITION.md`에 관찰값과 가설을 먼저 적고, `BUILD_ENV.md`에 환경을 기록합니다.

## 4. 개선과 근거 작성

`src/uart_config.c`, `frame_codec.c`, `stream_rx.c`, `stream_tx.c`를 개선합니다. 제공 Header·Host·고정 Test는 보존하고 private Helper는 추가할 수 있습니다. `tests/test_student.c`에는 서로 다른 경계 Test를 최소 2개 작성합니다.

`PROBLEM_DEFINITION.md`에는 현상·가설 구별·확정 원인·해결방안·검증을 작성합니다. `PROTOCOL.md`에는 Byte 규격과 상태 전이, 처리량 계산과 공식 근거를 남깁니다. `README.md`에는 최종 변경 요약을 기록합니다.

## 5. 같은 입력으로 재검증

```sh
make clean
make test > logs/test.log 2>&1
test_status=$?
printf '\nmake test exit code: %s\n' "$test_status" >> logs/test.log
cat logs/test.log
make demo > logs/demo.log 2>&1
demo_status=$?
printf '\nmake demo exit code: %s\n' "$demo_status" >> logs/demo.log
cat logs/demo.log
```

최종 Test와 Demo가 성공한 코드의 Log를 보관합니다. 변경 이후 이전 Log를 재사용하지 않습니다. 모든 실패와 코드 계약의 경계를 근본 원인 목록에 연결합니다. Host 검증을 실제 ASCLIN·Board 실행 결과로 적지 않습니다.

## 6. 자신의 Fork에 제출 변경 Push

```sh
cd ../../..
git add submissions/week07/YOUR_GITHUB_ID
git diff --cached
git commit -m "Improve week07 UART records with evidence and tests"
git push -u origin solve/week07-YOUR_GITHUB_ID
```

자기 제출 폴더 외 변경이나 PDF·ZIP·Build 산출물·SDK·인증 파일이 없는지 확인합니다. 구현과 검증을 구분한 의미 있는 Commit을 추가할 수 있으며 빈 Commit을 만들 필요는 없습니다.

## 7. 원본 저장소로 PR

- Base repository: `zinee-u/embedded-just-for-fun`
- Base branch: `main`
- Head repository: 자신의 Fork
- Compare branch: `solve/week07-YOUR_GITHUB_ID`
- 제목: `[07회차] YOUR_GITHUB_ID TC387QP UART 기록·Parsing 개선`

[PR 본문 양식](PR_BODY.md)을 실제 값으로 채웁니다. Files changed에는 자기 제출 폴더만 보여야 합니다. 자신의 Fork 내부 PR만 만들면 원본 저장소로 제출된 것이 아닙니다. Review에 따른 개선은 같은 Branch에 Commit하고 Push합니다.
