# 매주 과제 추가와 PR 운영

## 로컬 관리 위치

이 Git 저장소의 루트는 `firstclass/issues`이며 원격 저장소는 `zinee-u/embedded-just-for-fun`입니다. 상위 `firstclass` 전체를 올리지 않습니다.

- `firstclass/issues/weeks`: 공개할 문제와 개선 전 코드
- `firstclass/issues/submissions`: PR로 받은 학생별 개선안
- `firstclass/materials/weekNN`: 출제자가 별도로 관리하는 과제 문서와 해설

05회차는 출제자 요청에 따라 `weeks/week05/reference/`의 한국어 해설 코드와 실행용 Host·Test를 예외적으로 공개합니다. `starter/`의 결함은 보존하고 제출은 기존 `submissions/week05/<github-id>/` 경로를 사용합니다. 다른 회차의 해설과 비공개 자료까지 복사하지 않습니다.

06회차의 풀이 접근법은 `weeks/week06/APPROACH.md`로 공개합니다.

07회차는 `weeks/week07/APPROACH.md`와 `baseline/`의 원본 실행 Log를 과제와 함께 공개합니다. `starter/`의 네 Module은 개선 전 상태를 유지하며, 현상·원인·해결방안과 직접 실행한 검증을 `submissions/week07/<github-id>/`의 PR로 받습니다. 완성 참고 구현과 내부 검증 자료는 `materials/week07/answer/`에 유지합니다.

GitHub에는 코드와 Markdown 문서를 사용하고 PDF와 ZIP은 올리지 않습니다. PDF는 로컬 `materials/weekNN/`와 로컬 배포 ZIP에서만 관리하며, PDF와 ZIP은 Git으로 추적하거나 Push하지 않습니다.

`materials`와 개인정보가 담긴 수업자료는 이 저장소 바깥에 둡니다. 이 저장소의 로컬 커밋 이름·이메일은 출제자가 지정한 값으로 설정하며 다른 저장소의 전역 설정은 바꾸지 않습니다.

## 새 주차 추가

과제와 해설은 `firstclass/materials/weekNN/`에 함께 생성합니다. 문제 풀이 접근법 PDF는 과제 PDF의 파일명에서 `.pdf`를 뺀 뒤 `_문제_풀이_접근법.pdf`를 붙입니다. 예를 들어 `06회차_TC387QP_Interrupt_수집_개선_요청서.pdf`의 해설은 `06회차_TC387QP_Interrupt_수집_개선_요청서_문제_풀이_접근법.pdf`입니다. 생성 코드·로컬 README·로컬 배포 파일의 링크를 같은 이름으로 맞춥니다. 공개 README는 저장소의 Markdown 문서를 연결하며 로컬 PDF나 삭제한 PDF 경로를 다운로드 링크로 안내하지 않습니다. 해설의 공개 여부는 해당 회차에 대한 출제자 요청에 따릅니다.

1. 로컬 변경을 확인하고 `git pull --ff-only origin main`으로 병합된 제출물까지 받습니다.
2. `weeks/week03/starter/`처럼 수업 회차에 맞는 폴더를 만듭니다.
3. `templates/week/README.md`를 복사해 문제와 정확한 조건을 씁니다.
4. `starter`에는 개선 전 함수와 실습에 필요한 API·Build 설정·검증 도구를 넣습니다. 정답 구현과 출제자 해설을 넣지 않습니다. 4주차처럼 Host 대역을 제공할 경우 실제 MCU 실행과의 차이를 명시합니다.
5. 필요한 정상·경계·오류 입력을 `cases.csv` 등에 정리합니다.
6. 루트 `README.md` 과제 목록에 주차별 링크를 추가합니다.
7. PDF·ZIP·공개가 허용되지 않은 완성 참고 구현과 내부 검증 자료가 제외되었는지 확인하고, 올릴 파일만 지정해서 커밋한 뒤 `main`에 푸시합니다.

```sh
cd /path/to/firstclass/issues
git status --short
git pull --ff-only origin main
mkdir -p weeks/week03/starter
cp templates/week/README.md weeks/week03/README.md
# 문제를 작성하고 개선 전 코드를 starter에 저장합니다.
# 루트 README.md의 과제 목록에도 새 링크를 추가합니다.
git add weeks/week03 README.md
git diff --cached
git commit -m "Add week03 assignment starter"
git push origin main
```

아직 학생별 제출 폴더를 미리 만들 필요는 없습니다. 학생의 첫 PR에서 생성됩니다. 앞으로도 주차는 `week04`, `week05`처럼 늘립니다.

## 풀이 PR 검토

- 해당 학생의 `submissions/weekNN/<github-id>/`에만 변경이 있는지 봅니다.
- 증상뿐 아니라 근본 원인을 빠짐없이 분석했는지 확인합니다.
- 요구 조건과 함수 계약, 호출자 사용법이 일치하는지 봅니다.
- 원인별 반례와 정상·경계·오류 테스트, 실제 실행 환경이 있는지 확인합니다.
- 자신의 말로 설명한 이유와 선택한 방식의 단점도 검토합니다.
- 필요한 피드백은 PR에 남기고 같은 PR에서 보완하게 합니다.
- 검토가 끝난 풀이만 병합합니다. 학생 답안을 `weeks/.../starter`에 복사하지 않습니다.

이 구조는 경로와 제출 규칙으로 원본을 보존합니다. 저장소 권한이나 브랜치 보호 규칙을 자동 설정한 것은 아닙니다. 현재 자동 채점 또는 자동 병합은 없습니다.

## 문제를 정정해야 하는 경우

풀이와 별도의 커밋 또는 PR로 조건을 정정하고, 해당 주차 README의 변경 기록에 날짜·내용·영향을 적습니다. 이미 제출된 답안이 다른 조건을 기준으로 작성되었다면 그 차이를 검토에서 밝혀 주세요.

## 공개 답안에 대한 운영 선택

PR을 제출하는 순간 코드와 토론은 공개됩니다. 병합을 늦춰도 PR 자체는 보일 수 있습니다. 독립 풀이를 원하면 먼저 개인 답안을 만든 후 PR을 열도록 안내하거나 제출 시점을 함께 정하세요.
