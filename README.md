# InnerStat Project
이 프로젝트는 시스템과 하위 서비스들의 유지보수를 위한 모니터링 소프트웨어를 제작하는 프로젝트입니다.

## 기본 계획
### 클라이언트
- 클아이언트는 모니터링을 할 수 있는 소프트웨어로 
- C++ 언어로 제작하며 wxWidgets 라이러리를 이용하여 GUI를 제작
- 에이전트와 통신하며 시스템과 서비스틀의 상태를 표시
- 시스템을 OS, VM 등 세부적으로 나누어 적절하게 상태를 알 수 있도록 함
- 서비스들은 어떤 포트로 Listen하는지와 PID가 무엇인지 등 정보를 가짐

### 에이전트
- 에이전트는 ...

---
# 파일 구조
```
InnerStat
├── Agent
│   └── test.cpp
├── Client
│   ├── MainApp.cpp
│   ├── Canvas.h / .cpp
│   ├── Shape.h / .cpp
│   ├── Area.h / .cpp
│   ├── Node.h / .cpp
│   ├── Connection.h / .cpp
│   └── Port.h / .cpp
└── 
```

## Agent
### test.cpp
- 내용 없음

## Client

### MainApp.cpp (`MyApp`, `MainFrame` 클래스)
- `MyApp`: wxWidgets 애플리케이션 초기화 클래스
- `MainFrame`: UI 구성 및 이벤트 바인딩
  - 좌측: `wxTreeCtrl` + "Add Area" 버튼 포함 패널
  - 우측: `MainCanvas` (도형 배치 캔버스)
  - 상단 메뉴: 열기/저장 기능 포함

---

### Canvas.h / Canvas.cpp (`MainCanvas` 클래스)
- 도형(`Area`, `Node`)과 연결선을 표시하는 **시각화 캔버스**
- 주요 기능:
  - 도형 추가, 선택, 이동, 크기 조절
  - 포트 클릭을 통한 연결 생성
  - `wxTreeCtrl`과 연동된 도형 구조 트리
  - 마우스 기반 확대/축소 및 패닝
  - 도형 및 연결 정보의 저장/불러오기

---

### Shape.h / Shape.cpp (`Shape` 클래스)
- `Area`와 `Node`의 **공통 기반 클래스**
- 속성:
  - 위치, 크기, 포트, 선택 여부, 부모 도형 등
- 메서드:
  - `Draw`, `Contains`, `HitTestHandle`, `OpenProperty` 등
  - 포트 충돌 감지 및 선택 처리

---

### Area.h / Area.cpp (`Area` 클래스)
- 시스템 단위를 표현하는 도형 (예: OS, VM, Container 등)
- 기능:
  - 하위에 `Area` 또는 `Node` 포함 가능
  - 타입, 포트 수, 자식 도형 추가 등 속성 설정
  - 포트 시각화 및 연결 처리

---

### Node.h / Node.cpp (`Node` 클래스)
- 말단 서비스/프로세스를 나타내는 도형
- PID, 활성 상태(`active`), 과부하 상태(`overloaded`) 포함
- 기능:
  - 상태에 따른 색상 시각화
  - PID 및 포트 수 속성 편집 가능

---

### Connection.h / Connection.cpp (`Connection` 클래스)
- 두 포트를 연결하는 꺾인 선(엘보형 연결선)
- 포트 위치 계산을 통해 정확한 시각화 지원
- `Draw()` 메서드를 통해 선을 그림

---

### Port.h / Port.cpp (`Port` 클래스)
- 도형에 부착된 입출력 포인트
- 도형 내 상대 좌표를 기반으로 절대 위치 계산
- 시각적으로 원형 포트 및 포트 ID를 그림
- 히트박스 처리로 클릭 및 연결 감지 지원

---

# 진행상황

| 심볼    | 설명          | 심볼    | 설명          |
|---------|---------------|--------|--------------|
| 🟢✅   | 완료          | ⭐📌   | 중요 일정     |
| 🟡🚧   | 진행 중       | 🕒      | 일정 예정     |
| 🔵⏳   | 대기 중       |
| 🔴❌   | 실패 또는 취소 |

### Agent
- 기본 틀 제작

### Client
- Shape 자식관계
  - [x] Area와 Node 클래스 생성 🟢
  - [x] Area 내부에 자식 Area와 Node 생성 🟢
  - [] Shape의 위치를 상대적으로 바꾸기(부모 안에 위치) 🔵
  - [] Shape 서로 겹치지 않게 하기 🔵
- 포트 관련
  - [ ] 고유 포트 아이디 생성 🔵
- 커넥션 관련
- 코드 최적화