# 과제 저장소 작업 규칙

이 저장소는 주차별 개선 전 코드와 문제를 보존하고, 학생이 Pull Request로 개선안을 제출하는 공개 학습 저장소다.

- `weeks/weekNN/README.md`에 문제, `weeks/weekNN/starter/`에 수정 전 코드를 둔다. `weekNN`은 수업 회차다.
- 원본 코드에 의도된 결함을 자동으로 고치지 않는다. 학생 풀이 때문에 함수 원형이나 문제를 덮어쓰지 않는다.
- 학생 제출은 `submissions/weekNN/<github-id>/` 안에서만 변경한다. 다른 학생의 제출을 변경하지 않는다.
- 모든 근본 원인을 찾고 각 원인에 대해 실패 상황, 수정 이유, 검증을 설명한다. 주어진 조건 밖의 확장 문제는 구분한다.
- 함수 인터페이스는 주차별 문제에서 허용한 범위 내에서 자유롭게 설계한다. 한 가지 정답 API를 강제하지 않는다.
- 제출에는 개선 코드, 테스트, 설명과 실제 실행 결과를 포함한다. 실행하지 않은 결과를 실행했다고 보고하지 않는다.
- 출제자의 정답, 비공개 해설, 개인별 수업자료, 학생 개인정보, 토큰 및 인증 파일을 이 공개 저장소에 복사하지 않는다.
- 05회차만 예외: 출제자의 명시적 요청으로 `weeks/week05/reference/`에 한국어 해설 주석을 포함한 구현과 실행용 Host·Test를 공개한다. 원본 `starter/`는 유지하고, 풀이 PR은 기존처럼 `submissions/week05/<github-id>/`에 제출한다. 다른 회차의 해설이나 `materials` 전체를 공개하는 허용으로 확대하지 않는다.
- 06회차 문서 공개 예외(2026-10-19 갱신): 출제자의 요청으로 풀이 접근법은 `weeks/week06/APPROACH.md`로 공개한다. 과제와 풀이 접근법 PDF는 로컬 `materials/week06/`에서 관리한다. 전체 참고 구현과 내부 검증 Log는 `materials/week06/answer/reference/`에 유지한다. 풀이 PR은 `submissions/week06/<github-id>/`에 제출한다.
- 07회차 문서 공개 예외(2026-10-25): 출제자의 요청으로 `weeks/week07/APPROACH.md`를 과제 Markdown·공식 근거·제출 안내와 함께 공개한다. `baseline/`에는 원본 실행 Log, `starter/`에는 개선 전 네 Module과 Host Test·분석 양식을 둔다. 완성 참고 구현과 내부 검증 자료는 `materials/week07/answer/`에 유지하고 PDF·ZIP은 공개하지 않는다. 풀이 PR은 `submissions/week07/<github-id>/`에 제출한다.
- GitHub에는 코드와 Markdown 문서를 사용한다. PDF는 로컬 `firstclass/materials/weekNN/`와 로컬 배포 ZIP에서만 관리하며 Git으로 추적하거나 Push하지 않는다. ZIP도 공개 저장소에 넣지 않는다.
- 출제자가 새 과제를 요청하면 원본과 문제만 해당 주차에 추가하고 루트 과제 목록을 갱신한다. 해설은 별도 `firstclass/materials/weekNN/`에서 관리한다.
- 로컬에서 생성하는 문제 풀이 접근법 PDF의 파일명은 `<과제 PDF의 확장자를 뺀 파일명>_문제_풀이_접근법.pdf`로 통일한다. 예: `06회차_TC387QP_Interrupt_수집_개선_요청서_문제_풀이_접근법.pdf`. 공개 Markdown에서는 저장소의 Markdown 문서를 연결하며 로컬 PDF나 삭제한 PDF 경로를 다운로드 링크로 안내하지 않는다.
- 기존 주차 문제를 정정할 때는 변경 내용을 주차 README의 변경 기록에 남긴다. 학생이 제출한 전제 조건을 몰래 바꾸지 않는다.
- 코드 탐색은 codebase-memory-mcp의 search_graph, trace_path, get_code_snippet 등을 우선 사용한다. 미색인 저장소는 먼저 index_repository를 실행한다. 그래프 결과가 부족하거나 비코드 자료·문자열을 찾을 때만 파일 검색으로 보완한다.

이 저장소의 로컬 관리 위치는 `firstclass/issues`다. 상위 `firstclass` 전체를 GitHub에 올리지 않는다.
