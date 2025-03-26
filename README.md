# 개요
이 프로젝트는 [숫자 야구 게임]으로, Bulls and Cows게임을 원조로 하여 제한된 횟수 안에 목표 숫자를 맞추는 것을 목표로 하는 [멀티플레이 턴제] 게임입니다. 주요 기능으로는 '채팅'과 '턴 제어 기능'이 포함되어 있습니다. 각 플레이어는 제한된 횟수 안에 숫자를 입력하며, 모든 플레이어의 기회가 소진되면 게임이 리셋됩니다.

# 기능
## 1. 기본 기능
- [GameMode 서버 로직]: 채팅 기반으로 명령어를 입력하면 서버가 이를 해석해서 게임 로직을 처리합니다.
- [3자리 난수 생성 로직]: 게임 시작 시 서버에서 무작위 3자리 숫자를 만듭니다. 
- [판정 로직]: 서버가 생성한 숫자와 플레이어가 입력한 숫자를 비교하여 스트라이크, 볼, 아웃을 계산합니다. 
- [시도 횟수 및 상태 관리]: 플레이어(Host/Guest)각각 3번의 기회를 가집니다. 입력할 때마다 플레이어별 남은 시도 횟수를 갱신합니다.
- [턴 제어 기능]: 서버에서 랜덤으로 선턴을 부여하고 화면의 초록색 원 이미지로 자신의 턴 여부를 알 수 있습니다. 

## 2. 게임 메커니즘
- [판정 로직]: 자리수와 값이 같으면 S, 자리수는 다르지만 값이 존재하면 B, 둘 다 아니면 OUT으로 판정합니다.
- [승리, 무승부, 게임 리셋]: 3번 내에 3S를 맞춘 플레이어가 나오면 즉시 승리처리합니다. 플레이어가 아웃되었을 때, 남은 플레이어가 자동으로 승리합니다. 한 플레이어가 승리하거나 모두 기회를 소진하면 게임을 재시작해 숫자와 시도 횟수를 리셋합니다.


# 구현 과정
## 1. 프로젝트 설정
- Unreal Engine을 설치하고 새 프로젝트를 생성했습니다.
- 프로젝트 설정에서 C++ 코드 작성을 선택하고, 기본 템플릿을 사용했습니다.

# 2. 주요 클래스
- `NBGameMode.cpp`: 게임의 전반적인 흐름을 관리하는 클래스. 게임 규칙 적용, 라운드 시작 및 종료 등을 담당. NBGameState와 NBPlayerController를 제어.
- `NBGameState.cpp`: 게임의 현재 상태를 관리하는 클래스. 남은 플레이어 수, 점수, 라운드 진행 상황 등의 정보를 저장 및 동기화. Replicated 변수로 서버와 클라이언트 간의 게임 상태를 공유.
- `NBGameRulesLibrary.cpp`: 게임 규칙 관련 로직을 포함하는 정적 라이브러리. 점수 계산, 승패 조건 판별 등 공통 규칙을 함수로 제공. 
- `NBGenerateRandomNumberLibrary.cpp`: 랜덤 숫자 생성 관련 기능을 제공하는 정적 라이브러리. 특정 범위의 랜덤 값 생성을 처리.
- `NBPlayerController.cpp`: 플레이어 입력을 처리하고 서버와 상호작용하는 역할. 채팅, UI 조작 등의 기능을 포함. NBPlayerState와 연동하여 플레이어 정보를 관리.
- `NBPlayerState.cpp`: 개별 플레이어의 상태를 관리하는 클래스. UserID, 점수, 남은 기회, 현재 턴 여부 등의 정보를 저장 및 동기화.
- `NBChatWindow.cpp`: 게임 내 채팅 UI를 담당하는 클래스. NBPlayerController와 연동하여 메시지를 송수신 및 출력.

# 3. 기능 구현

## 3.1. [3자리 난수 생성 로직] 구현
[3자리 난수 생성 로직]을 구현하는 과정은 다음과 같습니다.
- [단계1]: 명세에 맞추어 조건을 확인합니다 : 0은 포함하지 않음, 중복 없음
- [단계2]: 각 자릿수의 숫자를 랜덤으로 뽑습니다.
- [단계3]: 같은 숫자가 있다면 [단계2]를 반복합니다.

## 3.2. [GameMode 서버 로직] 구현
[GameMode 서버 로직]을 구현하는 과정은 다음과 같습니다.
- [단계1]: ANBGameMode는 게임의 흐름을 제어하는 역할을 하며, 목표 숫자 설정, 메시지 비교, 플레이어 턴 설정 등의 기능을 담당합니다.
- [단계2]: ANBGameState의 SetGoalNumber()를 호출하여 목표 숫자를 설정합니다.
- [단계3]: 호스트는 "Host", 클라이언트는 "Guest"로 UserID를 설정합니다.
- [단계4]: 현재 접속한 모든 플레이어 컨트롤러를 가져와 2명 이상일 경우 진행합니다. ANBGameState의 CurrentTurnPlayer를 설정하여 현재 턴인 플레이어를 기록합니다. 플레이어 수가 부족할 경우 1초 후 다시 확인하도록 타이머를 설정한다.
- [단계5]: UNBGameRulesLibrary::CheckStrikeBall()을 호출하여 입력된 메시지[Message]와 목표 숫자[GoalNumber]를 비교합니다.

- ## 3.2. [스코어보드 UI 반영] 구현
게임의 진행 상태와 함께 목표 숫자(GoalNumber)를 UI에 반영하는 기능을 추가했습니다. 이를 통해 각 플레이어는 자신의 목표 숫자와 현재 진행 상황을 실시간으로 확인할 수 있습니다.
