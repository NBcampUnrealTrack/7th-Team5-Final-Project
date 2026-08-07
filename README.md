# Karon

<p align="center">
  <img alt="Engine" src="https://img.shields.io/badge/Unreal_Engine-5.7-0E1128?logo=unrealengine&logoColor=white" />
  <img alt="Platform" src="https://img.shields.io/badge/Platform-Windows-0078D6?logo=windows&logoColor=white" />
  <img alt="Genre" src="https://img.shields.io/badge/Genre-Action%20%2B%20Base%20Building-e85d04" />
  <img alt="Gameplay Ability System" src="https://img.shields.io/badge/System-GAS-8250df" />
  <img alt="UI" src="https://img.shields.io/badge/UI-CommonUI%20%2B%20UMG-1f6feb" />
</p>

Unreal Engine 5.7 기반 액션 + 베이스 빌딩 프로젝트입니다.  
이 문서는 `Source` 기준으로 프로젝트의 핵심 구조와 시스템 연결을 빠르게 파악할 수 있도록 정리한 **프로젝트 메인 README**입니다.

---

## 📌 프로젝트 요약

- **프로젝트 성격**: 액션 전투 + 베이스 빌딩 (타워 디펜스 / 팩토리 하이브리드)
- **기본 시작 맵**: 타이틀 화면 → 인게임 (`AKOTitleController` → `AKOGameMode`)
- **핵심 기술 축**: GAS, CommonUI/UMG, AI Behavior Tree, Data-Driven Asset, Niagara VFX, Motion Matching
- **메인 런타임 모듈**: `Source/Karon`
- **버전 관리**: Perforce (`ssl:mystisle.synology.me:51666`, 워크스페이스 `Karon_hyunseok`)

---

## 🧱 모듈 구성

### `Karon` (Runtime)

- 위치: `Source/Karon`
- 빌드 설정: `Source/Karon/Karon.Build.cs`
- 주요 의존성:
  - **Public**: `GameplayAbilities`, `GameplayTags`, `GameplayTasks`
  - **Public**: `CommonUI`, `CommonInput`, `UMG`, `EnhancedInput`
  - **Public**: `MotionWarping`, `MotionTrajectory`, `PoseSearch`
  - **Public**: `GMRouter`, `StructUtils`, `DeveloperSettings`, `Niagara`, `NiagaraAnimNotifies`
  - **Private**: `Slate`, `SlateCore`, `AIModule`, `NavigationSystem`, `SQLiteCore`, `LevelSequence`
- 주요 플러그인: `GameplayAbilities`, `GameplayStateTree`, `PoseSearch`, `MotionTrajectory`, `AnimationWarping`, `MetaHuman`, `CommonUI`, `GMRouter`(커스텀)

---

## 🗺️ 코드 도메인 맵

`Source/Karon/` 기준:

- `Game/`: 게임 모드, 플레이어 컨트롤러/스테이트, 카메라 매니저, 타이틀 컨트롤러, 발사체 풀 서브시스템, 세이브 데이터
- `Character/`: 캐릭터 계층 (Hero, Enemy, Boss), AI 컨트롤러, BehaviorTree 노드, 발사체
- `AbilitySystem/`: GAS 어트리뷰트 셋 7종, 어빌리티, 이펙트, 태그, 어빌리티 태스크, GameplayCue
- `Animation/`: 애니메이션 인스턴스, 노티파이, 애니메이션 타입 정의
- `Component/`: 이동, 빌딩, 인벤토리, 입력, 인터랙션, 맵 UI, 장비, 카메라 수정자
- `Building/`: 빌딩 기반 클래스, 컨베이어 벨트, 고스트 프리뷰, 그리드 비주얼
- `Subsystem/`: 에너지, 그리드, 컨베이어, 로드, 스킬, UI, GAS 글로벌, 퀘스트 가이드, 세이브, 텔레메트리 서브시스템
- `UI/`: HUD, 위젯 클래스, UI 서브시스템, 맵/인벤토리/팩토리/적 HP/스킬/Fog of War 위젯
- `Items/`: 아이템 정의, 아이템 라이브러리, 인벤토리 컴포넌트, 장비/무기 시스템
- `Skills/`: 스킬 서브시스템, 스킬 라이브러리, 스킬 데이터 구조
- `Data/`: 콤보 액션, 입력 설정, GrantSet, 무기 정의 DataAsset
- `MapActor/`: 월드 배치 액터 (드롭 아이템, 퀘스트 게이트, 텔레포트, 구역 감지, 레벨 시퀀스 매니저)
- `Debug/`: GAS 디버그 라이브러리 (에디터 전용)
- `Utility/`: 로그, 메시징 채널 타입, 공용 인터페이스, 범위 쿼리 헬퍼

---

## 🔄 런타임 흐름

### 1) 타이틀

- `AKOTitleController`가 타이틀 화면 UI와 입력을 담당
- 게임 시작 시 인게임 맵으로 전환

### 2) 인게임 초기화

- `AKOGameMode`가 게임 전반을 관리
- `AKOPlayerState`가 GAS(`UKOAbilitySystemComponent`) 및 모든 어트리뷰트 셋을 소유
- `AKOPlayerController`가 입력, 빌딩, 인벤토리 컴포넌트 허브 역할

### 3) 전투

- 플레이어: `AKOHeroCharacter` + `UKOHealthSet` / `UKOCombatSet` / `UKOMovementSet`
- 일반 적: `AKOBaseEnemy` + `AKOBaseEnemyAIController` + BehaviorTree
- 보스: `AKOBossBase` → `AKOBossChapter01` + `KOAIC_BossChapter01`
- 데미지 흐름: `UKOExecCalc_Damage` → `UKOHealthSet` → `OnHPChangedEvent` 델리게이트 → UI 반영

### 4) 베이스 빌딩

- `UKOGridBuildComponent`의 상태머신으로 배치/회전/파괴 모드 전환
- `UKOGridSubsystem`이 셀 크기 150cm 그리드 점유 관리
- `AKOBaseBuilding` 스폰 후 `BeginPlay`에서 에너지/컨베이어 서브시스템에 자체 등록

### 5) 플레이어 사망 처리

- PC 입력 비활성화 → `RespawnDuration`(기본 2s) 대기 → `FadeDuration`(기본 2s) 후 레벨 리로드

---

## ⚔️ 주요 시스템 상세

### 🎮 GAS (Gameplay Ability System)

- 어트리뷰트 소유권은 `AKOPlayerState`에 집중 (맵 전환/부활 후에도 유지)
- 어트리뷰트 셋 7종: `HealthSet`, `StaminaSet`, `CombatSet`, `MovementSet`, `GroggySet`, `GuardSet`, `AttributeSetBase`
- 어빌리티 등록: `UKOGrantSet` DataAsset으로 `ActiveAbilities` / `SubAbilities` / `GrantedEffects` 일괄 부여
- 크리티컬 히트: `FKOGameplayEffectContext`가 `IsCriticalHit()` 플래그를 보유, `UKOExecCalc_Damage`에서 판정

### 🧍 캐릭터 & 애니메이션

- `AKOHeroCharacter`: LockOn 컴포넌트, PreCMCTick 컴포넌트, CharacterTrajectory, MotionWarping 탑재
- Motion Matching: `PoseSearch` + `MotionTrajectory`로 자연스러운 이동 애니메이션 구현
- Gait 상태(Walk/Run/Sprint)는 캐릭터 내부에서 추적하고 AnimInstance에 브로드캐스트
- `UKOPreCMCTickComponent`: CMC보다 먼저 틱하여 `CharacterTrajectoryComponent` 1프레임 지연 방지

### 📷 카메라 수정자 시스템

- `AKOCameraManager`가 `TArray<UKOCameraModifier*>`를 Priority 순으로 순회하며 ControlRotation / ArmLength / FOV / BoomOffset 합산 적용
- 구현체: `UKOArmLengthOffsetByStateModifier`, `UKOYawFollowModifier`, `UKOSpeedFOVModifier`
- 새 카메라 효과는 `UKOCameraModifier`를 서브클래싱 후 `AKOCameraManager`의 `Modifiers` 배열에 등록

### 🗡️ 장비/무기 시스템

- 데이터(`UKOWeaponDefinition`) + 런타임(`UKOEquipmentComponent` + `AKOWeaponBase`) 분리 구조
- 장착 시 `GrantSet`으로 어빌리티 부여, 해제 시 `FKOAbilitySetHandles::RemoveFromASC()` 일괄 취소

### 🛡️ 패링/반격 시스템

- `KOGA_Guard`(방어) → 퍼펙트 가드 이벤트 → `KOGA_ParryAttack`(플레이어 반격 발동) → 적에게 `KOGA_Parried`(그로기 상태) 부여
- 슈퍼아머: `SelfEffects` 배열에 `GE_SuperArmor` 등록 → `State.Character.SuperArmor` 태그 부여 → `KOGA_HitReact` 차단

### 🌟 스킬 시스템

- `UKOSkillSubsystem`이 스킬 잠금/해제 상태 독립 관리 (어빌리티 실행과 분리)
- 실행 타입: `Active`(GA 직접 발동) / `ActiveExtension`(콤보 확장) / `PassiveStat`(GE 즉시 적용)

### 🤖 적 AI

- BehaviorTree 기반, `AKOEnemyCluster`가 포아송 디스크 샘플링으로 볼륨 내 배치 (전멸 후 15초 자동 리스폰)
- 퍼셉션 5감각(시각/데미지/청각/예측/팀)이 블랙보드 키를 갱신
- `BTTask_ActivateAbility`가 GAS 어빌리티를 AI에서 발동하는 브릿지 역할 (태그 이벤트 방식으로 완료 감지, Tick 폴링 금지)
- 공격 토큰 시스템: 근거리 2개 / 원거리 1개 토큰으로 동시 공격 수를 제한

### 👾 보스 AI

- `AKOBossBase`: 비동기 에셋 로딩, 페이즈 시스템(`PhaseRatio` 기본 0.5f), 기믹 시스템(`GimmickReadyRatios`) 탑재
- 기믹 구현체는 `Character/Enemy/Boss/Gimmick/`에 분리
- 챕터별 보스: `AKOBossChapter01` + `KOAIC_BossChapter01`

### ✨ Niagara VFX

- **패턴 A** (AnimNotifyState): 근접 공격 피격 시 일회성 이펙트 재생 (`KOEnemyAttackNotifyState`)
- **패턴 B** (NiagaraComponent 직접 부착): 발사체 Trail 등 지속 이펙트 — 풀 반환 시 `Deactivate()` 필수

### 🏗️ 베이스 빌딩

- `UKOGridSubsystem`: 셀 크기 150cm 그리드, Actor/셀 기준 듀얼 인덱스로 O(1) 쿼리
- `AKOGhostPreview`: 반투명 배치 미리보기 + 에너지 커버리지 오버레이 + 컨베이어 방향 화살표
- `bKeepBuildModeAfterPlacement`로 연속 설치 모드 지원

### 🔧 컨베이어 벨트 & 팩토리

- Pull(`IKOItemSource`) / Push(`IKOItemSink`) 방식 인터페이스로 아이템 흐름 연결
- `UKOConveyorSubsystem`이 모든 벨트 중앙 Tick 관리 + deferred 등록/해제 안전 처리
- `UKOFactoryProcessorComponent` 상태: Idle / Running / OutputBlocked

### ⚡ 에너지 시스템

- `UKOEnergySubsystem`: Producer/Consumer를 `FEnergyNetwork`로 클러스터링하여 공급률 계산
- `UKOEnergyProducerComponent`: 연료 소모(`FuelInBuffer` + `FuelDebt` 정수/소수 분리) + 출력량 관리

### 🖥️ UI 시스템

- `UKOUISubsystem`: 4계층 레이어 스택 (`GameLayer` / `GameMenuLayer` / `MenuLayer` / `ModalLayer`)
- 정적 헬퍼 `UKOUISubsystem::OpenWidget(this, Tag)` / `CloseWidget(this, Tag)`로 위젯 제어
- 새 위젯 등록 시 `Config/DefaultGame.ini [KOUISettings]`에 Tag → 클래스 매핑 필수

### 🌫️ Fog of War 시스템

- `AKOFogManager` (레벨 배치 Actor): 설정 carrier — `BeginPlay`에서 `UKOFogManagerSubsystem`에 설정 주입
- `UKOFogManagerSubsystem`: 실제 Tick/시야 판정/탐험 추적 전담. `IsLocationVisible()` / `IsLocationExplored()` API
- `UKOVisionComponent`: Actor에 붙이면 자동 등록/해제

### 📋 퀘스트 가이드 시스템

- `UKOQuestGuideSubsystem`: 선형 퀘스트 체인 관리 (14종 완료 조건 지원)
- Notify 패턴: 각 시스템이 이벤트 발생 시 `Notify*(...)` 호출 → 내부에서 목표 매칭 판정
- `AKOQuestGateActor`: 지정 퀘스트 완료 시 충돌체/이펙트를 비활성화해 통로 개방

### 💾 세이브 시스템

- `UKOSaveSubsystem`: `UKOSaveGame` 객체를 단일 슬롯으로 관리
- `UKOSaveGame`: 플레이어 Transform/스탯/인벤토리/무기, 빌딩 상태, 컨베이어/프로세서/발전기, 몬스터/보스 생사, Fog 픽셀 데이터 전체 직렬화
- `UKOSaveGameOption`: 그래픽·사운드 옵션 분리 저장

### 📨 메시징 채널 (GMRouter)

시스템 간 결합 없이 통신하는 모든 이벤트는 GMRouter 채널로 라우팅됨. 모든 타입은 `KOMessageTypes.h`에 정의.  
`IKOGMSInterface`를 상속하면 `Subscribe` / `Broadcast` / `Unsubscribe` 래퍼를 직접 사용 가능.

---

## 📁 주요 폴더 구조

```text
Source/
  Karon/
    AbilitySystem/
      Ability/
        Attack/
          Skill/
        Combat/
        Enemy/
        Movement/
        Status/
        Utility/
      Attribute/
      Cue/
      Effect/
      Tag/
        Data/ Event/ Gameplay/ Input/ Item/ Name/ Object/ Skill/ State/ UI/
      AbilityTask/
    Animation/
    Building/
      Conveyor/
    Character/
      Enemy/
        Boss/
          BehaviorTree/
          Gimmick/
          Projectile/
        BehaviorTree/
        Projectile/
      Hero/
    Component/
      Build/
      Camera/
        Modifier/
      Factory/
      Input/
      Inventory/
      Movement/
    Data/
      Character/
        Enemy/
      Equipment/
    Debug/
    Game/
      Save/
    Items/
      Equipment/
    MapActor/
    Skills/
    Subsystem/
      Telemetry/
    UI/
      Boss/
      Build/
      Craft/
      Enemy/
      GameOver/
      HUD/
      Interaction/
      Inventory/
      ItemTooltip/
      LevelSequence/
      Loading/
      Map/
        FOW/
      Notice/
      Option/
      Skill/
      Title/
    Utility/
      FunctionLibrary/
      Interface/
      Log/
      Messaging/
Config/
Content/
  Karon/
    AbilitySystem/
    Character/
    UI/
    Map/
```

---

## 🚀 시작하기

### 1) 프로젝트 열기

```powershell
# Visual Studio 2022에서 솔루션 열기
Karon.sln
```

또는

```powershell
# Unreal Editor에서 프로젝트 열기
Karon.uproject
```

### 2) 빌드

- **에디터**: Tools > Compile 또는 `Ctrl+Alt+F11` (Live Coding)
- **UBT CLI**:
  
  ```
  "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" KaronEditor Win64 Development "<프로젝트경로>\Karon.uproject"
  ```

### 3) 유료 에셋 동기화

```powershell
# 최초 셋업 (Python 필요)
Tools\Setup.bat

# 에셋 다운로드
Tools\DownloadAssets.bat
```

### 4) 실행 및 검증

- 에디터에서 C++ 컴파일 후 PIE(Play In Editor)로 전투/빌딩 시나리오를 검증합니다.
- 별도의 자동화 테스트 프레임워크 없이 PIE 직접 검증 방식을 사용합니다.

---

## ✅ 협업 체크리스트

- 기능 수정 시 C++ 로직 + 데이터 에셋/테이블 + UI 바인딩을 함께 점검
- 파일 편집 전 `p4 edit <파일경로>`로 반드시 체크아웃 (미처리 시 read-only 쓰기 오류)
- 에디터 전용 코드는 `#if WITH_EDITOR` 블록으로 감싸야 Shipping 빌드 링크 에러 방지
- `UNiagaraSystem` / `UNiagaraComponent` 사용 시 `Karon.Build.cs`에 `"Niagara"` 모듈 추가 필수
- 새 DataTable 추가 시 `Config/DefaultGame.ini [KODataRegistrySettings]`에 경로 등록 필수
- 새 UI 위젯 추가 시 태그 추가 → `DefaultGame.ini [KOUISettings]` 매핑 등록 → 인터랙션 연결 순서 준수
- 새 어트리뷰트 셋은 `UKOAttributeSetBase` 상속 후 `AKOPlayerState`에 등록
- 세이브 확장 시 `FKOSavedBuilding` 구조체 + `UKOSaveSubsystem` 직렬화/역직렬화 양쪽 동시 수정
- `UAbilitySystemGlobals` 상속 클래스는 `DefaultGame.ini [AbilitySystemGlobals]`에 반드시 등록
- PR 대상 브랜치는 `develop` (절대 `main`으로 직접 머지 금지)
- 머지 전: 에디터 PIE 정상 동작 확인 + 기존 기능 이상 없음 확인

---

## 📸 스크린샷

### 타이틀

![타이틀 스크린샷 자리](./Screenshot/Title.png)

### 전투

![전투 스크린샷 자리](./Screenshot/Combat.png)

### 보스

![보스 스크린샷 자리](./Screenshot/Boss.png)

### 베이스 빌딩

![베이스 빌딩 스크린샷 자리](./Screenshot/BaseBuilding.png)

### 스킬 트리

![스킬 트리 스크린샷 자리](./Screenshot/SkillTree.png)

---
