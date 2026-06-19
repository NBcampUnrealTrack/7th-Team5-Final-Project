# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

코드를 제외한 문서는 한글로 작성할것.
git 관련 추적은 아이디가 khs081215인것만 참조할 것.

---

## 행동 지침

**Tradeoff:** 속도보다 신중함을 우선시함. 단순 작업은 판단해서 적용.

- 구현 전 가정을 명시적으로 밝힘. 불확실하면 질문. 여러 해석이 가능하면 모두 제시.
- 요청된 것 이외의 기능 추가 금지. 단일 사용 코드에 추상화 금지.
- 관련 없는 코드 개선/리팩터링 금지. 기존 스타일 유지. 자신의 변경으로 생긴 고아 코드만 정리.
- 멀티스텝 작업 시 계획 명시: `1. [단계] → 검증: [확인 방법]`

---

## 프로젝트 개요

**프로젝트명:** Karon (카론)
**엔진:** Unreal Engine 5.7
**장르:** 액션 + 베이스 빌딩 (타워 디펜스/팩토리 하이브리드)
**주요 모듈:** `Karon` (Runtime), `Source/Karon/`

---

## 빌드 및 개발 명령어

### 빌드
- Visual Studio 2022: `Karon.sln` 열기 후 빌드.
- UE5 에디터: **Tools > Compile** 또는 Ctrl+Alt+F11 (Live Coding).

UBT CLI 빌드 (머신 경로 직접 지정 필요):
```
"C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" KaronEditor Win64 Development "<프로젝트 절대경로>\Karon.uproject"
```

### 새 C++ 클래스 추가
에디터: **Tools > New C++ Class** → 생성 후 `Karon.sln` 재오픈 필요.

### 클린 빌드
`Binaries/`, `Intermediate/` 삭제 → `.uproject` 우클릭 → **Generate Visual Studio project files** → 빌드.

### 자동화 테스트
별도 테스트 프레임워크 없음. 에디터 PIE(Play In Editor)로 직접 검증.

### 유료 에셋 동기화 (PaidAssets)
유료 에셋은 Git LFS가 아닌 Google Drive(`G:/내 드라이브/Unreal7th_PaidAssets`)로 별도 관리됨. 팀원 간 동기화는 Tools/ 스크립트를 사용:

```
Tools\UploadAssets.bat   ← 에셋 업로드 (Drive → 공유)
Tools\DownloadAssets.bat ← 에셋 다운로드 (공유 → 로컬)
Tools\Setup.bat          ← 최초 셋업 (Python 필요)
```

에셋 매니페스트(`UpdatedFileList.json`), 릴리스 노트(`UpdatedNotes.txt`), 아카이브(`PaidAssets_latest.zip`)는 Drive에 보관. 다운로드된 에셋은 `Content/PaidAssets/`에 위치.

---

## 게임 모드

`AKOGameMode : AGameModeBase` — 현재 단일 GameMode.

**플레이어 사망 처리 (`HandlePlayerDeath(DeathInstigator)`):**
1. PC 입력 비활성화
2. `RespawnDuration`(기본 2초) 대기 후 페이드 시작
3. `FadeDuration`(기본 2초) 후 `UGameplayStatics::OpenLevel`로 현재 레벨 리로드

두 값 모두 `EditDefaultsOnly`로 BP에서 설정 가능.

---

## PR 워크플로우

- **PR 대상 브랜치:** `develop` (절대 `main`으로 직접 올리지 않음)
- 머지 전 체크리스트: 에디터 PIE 정상 동작 확인 + 기존 기능 이상 없음 확인
- PR 유형: 기능 추가 / 버그 수정 / UI 변경 / 블루프린트 수정 / C++ 수정 / Level 수정 / 에셋 수정·추가 / 리팩토링

---

## 아키텍처

### 의존성 모듈 (Karon.Build.cs)

**Public:** `Core`, `CoreUObject`, `Engine`, `InputCore`, `GameplayAbilities`, `GameplayTags`, `GameplayTasks`, `EnhancedInput`, `MotionWarping`, `MotionTrajectory`, `PoseSearch`, `CommonUI`, `CommonInput`, `GMRouter`, `StructUtils`, `DeveloperSettings`, `Niagara`

**Private:** `Slate`, `SlateCore`, `UMG`, `NavigationSystem`, `AIModule`

**주요 플러그인:** `GameplayAbilities`, `GameplayStateTree`, `PoseSearch`, `MotionTrajectory`, `AnimationWarping`, `MetaHuman`, `CommonUI`, `GMRouter`(커스텀), `UEGitPlugin`(커스텀, Git LFS 2)

---

### 핵심 시스템 간 관계

#### 1. Gameplay Ability System (GAS)

**어트리뷰트 소유권 — PlayerState가 소유:**
`AKOPlayerState`가 모든 어트리뷰트 셋을 소유. 캐릭터는 PlayerState의 참조만 가짐. 맵 전환/부활 시에도 어트리뷰트가 유지되는 이유.

```
AKOPlayerState (IAbilitySystemInterface) → UKOAbilitySystemComponent
                                         → UKOHealthSet
                                         → UKOStaminaSet (영웅만)
                                         → UKOCombatSet
                                         → UKOMovementSet
```

`AKOCharacterBase` / `AKOBaseEnemy` / `AKOBossBase`는 PlayerState의 ASC를 참조.

**어트리뷰트 셋 5종:**
- `UKOHealthSet`: Health, MaxHealth + Damage/Healing meta 어트리뷰트
- `UKOStaminaSet`: Stamina, MaxStamina + StaminaDrain/StaminaRegen meta
- `UKOCombatSet`: AttackPower, Defense, AttackSpeed
- `UKOMovementSet`: MaxWalkSpeed, MaxAcceleration, BrakingDecelerationWalking, GroundFriction, MaxWalkSpeedCrouched, JumpStrength
- `UKOAttributeSetBase`: 추상 베이스. `PostAttributeChange()` 가상 함수 + `AdjustCurrentForMaxChange()` 헬퍼 제공

`BindMovementSet()`으로 이동속도/점프력 어트리뷰트 변경 시 캐릭터 이동 파라미터 자동 반영.

**어빌리티 등록 방식 — 영웅/적/보스 공통 GrantSet:**
- **영웅/일반 적 공통:** `UKOGrantSet` DataAsset — `ActiveAbilities`, `PassiveAbilities`, `GrantedEffects` 3개 배열을 한 번에 부여. `AbilitySystemComponent->GiveGrantSet()` 호출. `GiveToAsc()` 반환값 `FKOAbilitySetHandles`로 나중에 일괄 취소 가능(`RemoveFromASC()`). 각 적 타입(Drone/Tanker/Warrior)마다 `DA_KO~GrantSet.uasset`이 별도 존재.
- **보스:** `AKOBossBase::ApplyAbilities()` — `UKOBossDataAsset`에서 어빌리티 로드.

> ⚠️ `AKOBaseEnemy::DefaultAbilities` TArray + `GiveDefaultAbilities()` 방식은 PR #78에서 GrantSet으로 전환돼 삭제됨. 구버전 코드 참조 금지.

**데미지 흐름:** `UKOExecCalc_Damage` → `UKOHealthSet` 어트리뷰트 변경 → `AKOBaseEnemy::OnHitCallback()` → `FOnUIChangeEvent OnHPChangedEvent` 델리게이트(ProgressPercent, Damage) 브로드캐스트 → `UKOEnemyHPBar::OnHPChanged()` (UI 반영)

**`UKOGameplayAbilityBase` 헬퍼:** 모든 게임플레이 어빌리티는 이 클래스를 상속. 제공 API:
- `ApplyEffectToSelf` / `ApplyEffectToTarget` — GE 단순 적용
- `ApplyEffectSetByCallerToSelf` / `ApplyEffectSetByCallerToTarget` — SetByCaller 데이터 태그 기반 적용
- `GetAvatarCharacter`, `GetAvatarController`, `GetASC` — 액터/컴포넌트 접근자
- `HasMatchingTag`, `HasAllTags`, `HasAnyTags` — 태그 쿼리

**핵심 파일:** `Source/Karon/AbilitySystem/`, `Source/Karon/Character/KOCharacterBase.h`

---

#### 2. 캐릭터 & 애니메이션

**캐릭터 계층:**
```
AKOCharacterBase (UKOCharacterMovementComponent 사용)
  ├─ AKOHeroCharacter
  │  ├─ USpringArmComponent + UCameraComponent
  │  ├─ UKOLockOnComponent: 락온 대상 추적
  │  ├─ UKOPreCMCTickComponent: CMC 이전/이후 타이밍 후크
  │  ├─ UCharacterTrajectoryComponent: PoseSearch 모션 매칭용 궤적 예측
  │  └─ UMotionWarpingComponent: 루트모션 워핑 (적에게 거리 접근 등)
  ├─ AKOBaseEnemy (일반 적)
  └─ AKOBossBase  (보스 — AKOBaseEnemy와 무관한 별도 계층)
```

`UKOCharacterMovementComponent`: `UCharacterMovementComponent` 상속, `OnMovementModeChanged` 오버라이드. 모든 캐릭터(`AKOCharacterBase`)에 사용.

Gait 상태(Walk/Run/Sprint)는 캐릭터 내부 `CurrentGait`/`PreviousGait`로 추적 → `UpdateGait()`이 AnimInstance에 브로드캐스트. 애니메이션 타입은 `KOAnimationTypes.h` 집중 정의 (`ELocomotionDirection`, `EGait`, `EHipFaceDirection`, `ERootYawOffsetMode`).

모션 매칭: `PoseSearch` + `MotionTrajectory`

**핵심 파일:** `Source/Karon/Character/Hero/KOHeroCharacter.h`, `Source/Karon/Animation/`, `Source/Karon/Component/Movement/KOCharacterMovementComponent.h`

---

#### 3. 장비/무기 시스템

**패턴:** 데이터(Definition) + 런타임(Component + Actor) 분리. GAS와 통합.

- `UKOWeaponDefinition` (DataAsset):
  - `SlotTag`, `WeaponTypeTag`: 애니메이션 레이어 전환 기준
  - `EquipSocket`(예: `hand_r`), `UnEquipSocket`(예: `spine_02`): 장착/해제 소켓
  - `WeaponABP_Sheathed` / `WeaponABP_Carrying` / `WeaponABP_Combat`: 상태별 AnimBlueprint 클래스
  - `GrantedSet` (`UKOGrantSet`): 장착 시 부여할 어빌리티·이펙트 묶음
  - Draw/Sheathe 몽타주

- `UKOEquipmentComponent` (캐릭터 소유):
  - `EquipWeapon(UKOWeaponDefinition*)`: 무기 Actor 스폰, 소켓 Attach, GAS GrantSet 부여
  - `UnequipWeapon()`: 무기 제거, GAS 어빌리티 취소 (`FKOAbilitySetHandles::RemoveFromASC()`)
  - `DrawWeapon()` / `SheatheWeapon()`: 슬롯 상태(Hand/Holster) 전환 + 몽타주 재생
  - `CurrentWeaponConfig` + `CurrentWeaponActor` (`AKOWeaponBase`): 현재 장착 무기 추적

- `AKOWeaponBase`: 스폰되는 무기 Actor. `UStaticMeshComponent` 포함, Definition으로 초기화.

**핵심 파일:** `Source/Karon/Items/Equipment/`, `Source/Karon/Data/Equipment/`

---

#### 4. 스킬 시스템

스킬 잠금/해제 상태 관리는 어빌리티 실행과 분리됨.

- `UKOSkillComponent` (캐릭터 소유):
  - `TMap<FName, EKOSkillState>`: 스킬별 상태(Locked / Available / Unlocked) 추적
  - `TryUnlockSkill(FName)`: 선결 조건 검사 후 해제, 연쇄 스킬 재평가
  - `ArePrerequisitesMet(const FKOSkillRow&)`: DataTable 행의 선결 조건 검사
  - `IsUnlocked(FName)`, `GetSkillState(FName)`: 쿼리 API

- `UKOSkillLibrary`: Blueprint Function Library. `GetSkillRow(WorldContext, SkillId)` — LoadSubsystem에서 `FKOSkillRow` 조회.

- 데이터 구조: `FKOSkillRow` (DisplayName, Icon, prerequisites), `FKOSkillExecutionRow` (실제 발동 어빌리티/이펙트)

**핵심 파일:** `Source/Karon/Skills/`, `Source/Karon/Component/Skill/`

---

#### 5. 적 AI

**일반 적:** `AKOBaseEnemy` + `AKOBaseEnemyAIController`(베이스: `ADetourCrowdAIController`) → BehaviorTree 기반

**스폰:** `AKOEnemyCluster` — `EnemyMap`(클래스→수량)을 읽어 `UBoxComponent` 볼륨 내 포아송 디스크 샘플링으로 배치. 파라미터: `MinDistanceBetweenEnemies`(기본 300.f), `MaxAttemptsPerPoint`(기본 30).

**AI 퍼셉션 5개 감각 → 블랙보드 키 처리:**

| 감각 | 조건 | BB 처리 |
|---|---|---|
| `UAISense_Sight` | 감지 성공 | `TargetActor` 설정 + `OnBattleChanged(true)` |
| `UAISense_Sight` | 감지 소실 | `TargetActor` 클리어 + `OnBattleChanged(false)` + Prediction 예측(0.25s) |
| `UAISense_Damage` | 피격 | `TargetActor` 설정 + TeamEvent 브로드캐스트(`TeamEventDistance` 반경) |
| `UAISense_Hearing` | 청각 | `DetectLocation` 설정 |
| `UAISense_Prediction` | 예측 마커 | `DetectLocation` 설정 |
| `UAISense_Team` | 동료 팀 이벤트 | `TargetActor` 설정 |

- `BTTask_ActivateAbility`: GAS 어빌리티를 AI에서 발동하는 브릿지. `TryActivateAbilitiesByTag()` 대신 `GetActivatableGameplayAbilitySpecsByAllMatchingTags()` → `FMath::RandRange()` → `TryActivateAbility(Handle)` 패턴으로 단일 랜덤 선택. GrantSet에 같은 태그의 GA를 여러 개 등록하면 자동으로 선택지가 늘어남.
- EQS: `PlayerEnvQueryContext`로 플레이어 위치 쿼리
- 애님 노티파이: `KOEnemyAttackNotifyState`(공격 판정), `KOEnemyDeadStopNotify`(사망 처리)
- 델리게이트: `OnCharacterHit`, `OnEnemyDead`(BlueprintAssignable), `OnCharacterReset`, `OnHPChangedEvent`, `OnBattleEvent` → AIController에서 구독

**블랙보드 키 상수** (`AKOBaseEnemyAIController`):

| 키 이름 | 타입 | 용도 |
|---|---|---|
| `bIsDead` | Bool | 사망 상태 |
| `bIsHit` | Bool | 피격 상태 |
| `bIsMontageEnd` | Bool | 몽타주 종료 |
| `bIsLongRange` | Bool | 원거리 적 여부 |
| `SelfActor` | Object | 자기 자신 Actor |
| `AttackRadius` | Float | 공격 반경 |
| `Speed` | Float | 이동속도 |
| `StrafeSpeed` | Float | 스트레이프 속도 |
| `EnemyAttackDelayTime` | Float | 공격 딜레이 |
| `bCanAttack` | Bool | 공격 가능 여부 |
| `TargetActor` | Object | 추적 대상 |
| `DetectLocation` | Vector | 청각/예측 감지 위치 |

`SetAI()` 함수로 BehaviorTree 실행 및 위 파라미터를 한 번에 블랙보드에 설정. 개별 BB 키 직접 쓰기 금지. `TeamEventDistance`(기본 1500.f) 범위 내 동료 적에게 전투 이벤트 전파.

**핵심 파일:** `Source/Karon/Character/Enemy/KOBaseEnemy.h`, `Source/Karon/Character/Enemy/KOBaseEnemyAIController.h`

---

#### 6. 보스 AI

**`AKOBossBase : AKOCharacterBase`** — `AKOBaseEnemy`와 무관한 별도 계층. `Character/Enemy/Boss/` 하위에 위치.

- **비동기 로딩:** `StartAsyncLoad(UKOBossDataAsset*)` 호출 → `TSharedPtr<FStreamableHandle>`로 에셋 스트리밍 → 완료 시 `OnBossInitialized()` 가상 함수 호출.
- **페이즈 시스템:** `PhaseRatio`(기본 0.5f) — HP가 해당 비율 이하로 떨어지면 `OnPhaseChanged(NewPhase)` 가상 함수 호출. `bPhase2Triggered`로 중복 방지.
- **기믹 시스템:** `GimmickReadyRatios`(기본 `{0.5f, 0.15f}`) — HP가 각 비율 이하로 떨어질 때마다 순서대로 기믹 발동. `FiredGimmickRatios`로 중복 방지. 그로기 진입/종료는 `OnGroggyBegin()` / `OnGroggyEnd()` 가상 함수 오버라이드.
- **델리게이트:** `OnBossReady`, `OnBossDetectedPlayer`, `OnBossDeathAnimEnd`, `OnBossDied` → AIController(`KOAIC_BossChapter01`)에서 구독.
- **Chapter01 보스:** `AKOBossChapter01 : AKOBossBase`, AI: `KOAIC_BossChapter01 : AAIController`, BT Tasks: `BTService_BossCh01_AttackCheck`, `BTService_BossDetectPlayer`, `BTTask_BossActiveAbility`

**핵심 파일:** `Source/Karon/Character/Enemy/Boss/KOBossBase.h`

---

#### 7. 베이스 빌딩 시스템

**건설 상태 흐름:**
```
EnterBuildMenuMode()                      → State: None → BuildMenu
StartBuildModeWithId(FactoryId)          → State: BuildMenu → Placing
  LoadSubsystem에서 FKOFactoryRow 로드
  AKOGhostPreview 스폰 (미리보기)

RotatePlacementPreview(Direction)         → CurrentRotationStep 0~3 (90° 단위)
                                            코너 벨트는 bCornerFlipPlacement도 토글 (8방향)

RequestBuild()                            → State: Placing → None (또는 Placing 유지)
  GridSubsystem.CanBuildArea() 검증
  AKOBaseBuilding 스폰
  InitializeBuildingData(FactoryId) 호출
  BeginPlay에서 빌딩이 Energy/Conveyor 서브시스템에 자체 등록
  bKeepBuildModeAfterPlacement=true면 Placing 상태 유지 (연속 설치)

StartDestroyMode() / RequestDestroy()     → State: Destroying → None
  RefundStoredItems() (벨트 화물·가공 버퍼·연료를 인벤토리 환급)
  GridSubsystem.FreeAreaByActor()
```

벨트 배치 후 `TryQueueBeltConnect()` → 인접 공장 포트 수집 → `OpenNextBeltConnectPopup()` 순차 팝업(OnDeactivated마다 재귀 호출) → `BindToMachinePort()` 바인딩.

- `UKOGridSubsystem` (WorldSubsystem): 셀 크기 100.0f, 기본 10×10, 원점 (400,300,0)
  - 듀얼 인덱스: `TMap<AActor*, FKOGridOccupiedArea>` (Actor → 점유 영역) + `TArray<FKOGridCellData>` (셀 → Actor) — Actor 기준 O(1) 삭제, 셀 기준 O(1) 인접 쿼리 동시 지원
  - `CanBuildAt()`, `CanBuildArea()`, `OccupyArea()`, `FreeAreaByActor()`, `GetOccupyingActorAt()`
  - `FKOGridOccupiedArea`: 앵커 위치(`FIntPoint`) + 크기(`FIntPoint`)

- `UKOGridBuildComponent` (PlayerController 소유): 빌드 모드 상태머신 + GMRouter `FKOBuildModeChangedMessage` 브로드캐스트
- `AKOBaseBuilding`: `IKOInteractableInterface` 구현, `InitializeBuildingData(FName FactoryId)`로 건설 완료 후 초기화

**핵심 파일:** `Source/Karon/Subsystem/KOGridSubsystem.h`, `Source/Karon/Component/Build/KOGridBuildComponent.h`, `Source/Karon/Building/`

---

#### 8. 컨베이어 벨트 & 팩토리 프로세서

**아이템 흐름 인터페이스:**
- `IKOItemSource`: `PeekOutputItem()` / `PopOutputItem()` — Pull 방식
- `IKOItemSink`: `CanAcceptItem()` / `PushItem()` — Push 방식
- 구현체: `AKOConveyorBelt`, `UKOFactoryProcessorComponent`

**`AKOConveyorBelt`:**
- 슬롯 큐: `Slots[]` — 인덱스 0 = 헤드(입력), Last = 테일(출력)
- Shape: `EKOBeltShape::Straight` / `Corner` (90° 회전형)
- `TryResolveCornerFlipFromNeighbors()`: 인접 머신/벨트에서 방향 자동 추론
- 머신 포트 바인딩: `BindToMachinePort(Machine, Slot)` — 1 벨트 = 1 (Machine, Kind, PortIndex) 슬롯
- 아이템 시각화: `UInstancedStaticMeshComponent`(ISM) per 아이템 종류. `FKOItemRow::WorldMesh` → `ItemMesh` → 폴백 큐브
- `MoveAccumulator` + `SlotsPerSecond`: 프레임 단위 부드러운 슬롯 이동

**`UKOEnergyProducerComponent`** (발전기 컴포넌트):
- **인터페이스:** `IKOEnergyProducer` + `IKOItemSink`
- `FuelInBuffer`(int32) + `FuelDebt`(float): 연료 보유량 (정수 + 소수 분리)
- `InitializeFromRecipe()`: 레시피에서 `BurnRatePerSecond`, `PowerPerFuelUnit`, `AcceptedFuelItemId` 로드
- `TryInsertFuel(ItemId, Count)` / `TryExtractFuel(Count)` / `RestoreFuelBuffer()`: 연료 조작
- `GetBurnProgress()`: 연료 1단위 소모 진행도 (0~1, UI ProgressBar용)
- `GetCurrentOutputPerSecond()`: 직전 틱 실제 공급량 (수요/연료 반영)
- `GetMaxOutputPerSecond()`: `BurnRate × PowerPerFuel` (이론 최대값)
- `MaxFuelBuffer`: 최대 적재량 (기본 999)

**`UKOFactoryProcessorComponent`:**
- 상태: `EKOFactoryState` — Idle / Running / OutputBlocked
- `InputBuffer` / `OutputBuffer`: 아이템 종류별 수량 (`TMap<FName, int32>`)
- `SetSelectedRecipe()`: 레시피 수동/자동 선택
- `IKOEnergyConsumer` 구현: 에너지 부족 시 Running 중단
- `Progress`: 사이클 진행도 (UI 표시용)

**`UKOConveyorSubsystem`** (TickableWorldSubsystem):
- 모든 벨트의 중앙 Tick 디스패처
- `TQueue<TFunction<void()>> PendingActions`: Tick 도중 벨트 등록/해제를 안전하게 처리 (deferred 실행)
- `IsSlotBound(Machine, Kind, PortIndex)`: 글로벌 포트 점유 쿼리 (UI 피드백용)

**핵심 파일:** `Source/Karon/Building/Conveyor/`, `Source/Karon/Component/Factory/`, `Source/Karon/Subsystem/`

---

#### 9. 에너지/팩토리 서브시스템

- `UKOEnemyDataSubsystem` (GameInstanceSubsystem): 에너미 스탯/스킬 데이터 캐시
  - `GetEnemyData(FEnemyNameLevelInfo)`: EnemyNameTag + Level 복합 키로 `FEnemyInfo` 반환
  - `GetSkillData(FEnemySkillInfo)`: EnemyNameTag + SkillTag 복합 키로 스킬 수치 반환
  - `FEnemySkillInfo` / `FEnemyNameLevelInfo` 구조체는 TMap 키로 사용되므로 `GetTypeHash` 오버로딩 필수

- `UKOProjectilePoolSubsystem` (WorldSubsystem): 발사체 오브젝트 풀 관리
  - 풀 크기 100, 초기 스폰 위치 (-2000,-2000,-1000) — Tick 충돌 방지용 오프스크린 위치
  - `GetProjectile()` / `ReturnToPool(AActor*)` API

- `UKOEnergySubsystem` (TickableWorldSubsystem): Producer/Consumer 등록 패턴
  - `FEnergyNetwork` (내부 구조체): 커버리지 겹침으로 Producer/Consumer를 네트워크로 클러스터링
  - `RebuildNetworks()`: 네트워크 토폴로지 동적 재구성
  - `GetConsumerNetworkProductionRate(Consumer)`: 소속 네트워크의 공급 비율 반환
  - 커버리지 밖 Consumer는 `UncoveredConsumers`로 별도 추적 (공급률 = 0)
- `UKOLoadSubsystem` (GameInstanceSubsystem): DataRegistry 활용, 팩토리·아이템·장비·스킬 데이터 로드
  - 소프트 레퍼런스 + 강한 참조 캐시 (`LoadedTables[]`)로 GC 방지
  - Tag 역방향 인덱스: `ItemTagToId`, `EquipmentTagToId`
  - 주요 API: `FindItemRow()`, `FindFactoryRow()`, `FindRecipeRow()`, `ResolveItemIcon()`, `ResolveBuildingClass()`, `GetBuildableFactoryIds()`, `FindItemIdByTag()`, `IsEquipmentItem()`
- 빌딩↔서브시스템: `BeginPlay`에서 컴포넌트가 서브시스템에 자신을 등록

**핵심 파일:** `Source/Karon/Subsystem/`

---

#### 10. UI 시스템

- `UKOUISubsystem` (LocalPlayerSubsystem): 4계층 레이어 스택

| 레이어 | 용도 |
|---|---|
| `GameLayer` | HUD (항상 표시, 최하위 z-order) |
| `GameMenuLayer` | 인벤토리, 빌드 메뉴, 팩토리 패널 (게임 시간 흐름) |
| `MenuLayer` | 일시정지/설정 (게임 시간 정지) |
| `ModalLayer` | 확인 다이얼로그 등 (최상위 z-order) |

각 레이어 = `UCommonActivatableWidgetStack` 컨테이너. RootLayout의 `NativeConstruct()`에서 UISubsystem에 등록.

**정적 헬퍼 사용법 (권장):**
```cpp
UKOUISubsystem::OpenWidget(this, KOGameplayTags::UI_Widget_Inventory);
UKOUISubsystem::CloseWidget(this, KOGameplayTags::UI_Widget_Inventory);
```

- `UKOActivatableWidget` 기반 CommonUI 활성화/비활성화 생명주기
- 레이아웃: `UKOInGameRootLayout` / `UKOTitleRootLayout`
- 적 HP바처럼 월드 공간에 표시되는 UI는 `UWidgetComponent`로 캐릭터에 붙임 (`EnemyHPBarWidgetComponent`)
- 위젯 C++ 멤버를 BP와 연결할 때는 `UPROPERTY(meta=(BindWidget))`을 사용. 위젯 클래스는 `UCommonUserWidget` 상속.

**`UKOEnemyHPBar` 타이머 상수** (하드코딩 값, 수정 시 주의):
- `HPInterpolationTime`: 1.0s / `HPInterpolationInterval`: 0.05s / `DamageDelayMaxTime`: 2.0s / `BattleDelayMaxTime`: 5.0s

**핵심 파일:** `Source/Karon/UI/KOUISubsystem.h`, `Source/Karon/UI/`

---

#### 11. 입력 파이프라인

`AKOPlayerController` → `UKOInputComponent` → `UKOInputConfig`(데이터 에셋) → GameplayTag → GAS 어빌리티 발동

- `DefaultIMC` (일반 게임플레이) / `BuildIMC` (빌딩 모드) 두 Input Mapping Context
- `FKOBuildModeChangedMessage` GMRouter 구독 → `OnBuildModeChanged()`에서 `UEnhancedInputLocalPlayerSubsystem`으로 IMC 자동 교체
- PlayerController 소유 컴포넌트: `UKOInteractionComponent`, `UKOGridBuildComponent`, `UKOBuildUIComponent`, `UKOInventoryComponent`, `UKOMapUIComponent`, `UKOSkillComponent`

> TODO: 공장/전투 맵 분리 시 컨트롤러를 두 개로 관리하는 방향 검토 중 (`KOPlayerController.h` 주석 참고). `UKOSkillComponent`는 `LocalPlayerSubsystem`으로 전환 예정.

**핵심 파일:** `Source/Karon/Game/KOPlayerController.h`, `Source/Karon/Component/Input/KOInputComponent.h`

---

#### 12. 인터랙션 시스템

- `IKOInteractableInterface`: `CanInteract`, `OnInteract`, `GetInteractionPrompt`
- `UKOInteractionComponent` (PlayerController 소유): 범위 내 Interactable 감지 및 입력 처리

---

#### 13. 아이템 시스템

- `UKOItemLibrary`: DataRegistry 래퍼, 아이템 데이터 조회
- `FKOItemSlot`: 아이템 ID + 수량 쌍

**`UKOInventoryComponent`** (PlayerController 소유):
- `MaxSlots`(기본 40), `AcceptedItemsQuery`(FGameplayTagQuery), `bAcceptFactories`(기본 true)
- `TryAddItem(EKOSlotKind, ItemId, Count)` / `TryRemoveItem(ItemId, Count)`
- `RemoveAtSlot(Index, Count)`: 슬롯 인덱스 기반 차감, 실제 차감량 반환
- `SwapSlots(A, B)`: 같은 ItemId면 자동 머지
- `SplitStack(Index, Count)` / `MergeAllStacks()`
- `GetCountOf(ItemId)` / `HasEnoughItems(ItemId, Count)` / `GetSlotByIndex(Index)`
- 변경 시 `Data.Message.Inventory.Changed` GMRouter 메시지 브로드캐스트

**핵심 파일:** `Source/Karon/Items/`, `Source/Karon/Component/Inventory/KOInventoryComponent.h`

---

#### 14. 메시징 채널 (GMRouter)

시스템 간 결합 없이 통신하는 모든 이벤트는 GameplayMessage 채널로 라우팅됨:

| 채널 태그 | 페이로드 구조체 | 용도 |
|---|---|---|
| `Data.Message.Inventory.Changed` | `FKOInventoryChangedMessage` | 아이템 수량 변경 |
| `Data.Message.Factory.StateChanged` | `FKOFactoryStateChangedMessage` | 팩토리 활성/진행도 |
| `Data.Message.UI.PushLayerRequest` | `FKOUIPushLayerRequest` | UI 레이어 푸시 |
| `Data.Message.Building.Interacted` | `FKOBuildingInteractedMessage` | 빌딩 인터랙션 |
| `Data.Message.Build.QuickSlotChanged` | `FKOBuildQuickSlotChangedMessage` | 빌드 퀵슬롯 아이템 변경 |
| `Data.Message.Build.QuickSlotSelectionChanged` | `FKOBuildQuickSlotSelectionChangedMessage` | 퀵슬롯 선택 인덱스 변경 |
| `Data.Message.Build.ModeChanged` | `FKOBuildModeChangedMessage` | 빌드 모드 상태 전환 |
| `Data.Message.Producer.FuelChanged` | `FKOProducerFuelChangedMessage` | 에너지 생산자 연료 변경 |
| `Data.Message.Processor.Changed` | `FKOProcessorChangedMessage` | 팩토리 프로세서 상태 변경 |

모든 메시지 타입은 `Source/Karon/Utility/Messaging/KOMessageTypes.h`에 정의. Actor 참조는 `TWeakObjectPtr` 사용.

---

### Blueprint ↔ C++ 클래스 대응

C++ 클래스는 `BP_` 접두사를 가진 블루프린트 에셋으로 확장됨:

| C++ 클래스 | Blueprint 에셋 위치 |
|---|---|
| `AKOBaseEnemy` | `Content/Karon/Character/Enemy/BP_TestEnemy.uasset` |
| `AKOBaseEnemy` (스트레이프형) | `Content/Karon/Character/Enemy/BP_StrafeTestEnemy.uasset` |
| `AKOBaseEnemy` (스트레이프형 신규) | `Content/Karon/Character/Enemy/BP_StrafeEnemy.uasset` |
| `UKOEnemyHPBar` | `Content/Karon/UI/Enemy/WBP_KOEnemyHPBar.uasset` |

에셋은 `Content/Karon/` 하위에 시스템별로 분리됨: `Character/`, `UI/`, `Map/`, `AbilitySystem/` 등.

---

### 주요 데이터 구조

**`FKOFactoryRow` (DataTable):** DisplayName, InputSlots, OutputSlots, BaseCycleSeconds, BuildingClass(소프트 레퍼런스), GridSize, PlacementZOffset, FactoryCategoryTag, Icon, SortOrder

**`FKOItemRow` (DataTable):** DisplayName, Categories, MaxStack, Icon(소프트 레퍼런스), WorldMesh(컨베이어 시각화용)

**`FEnemyStat`:** MaxHP, Def, Atk, MoveSpeed, DetectRange, AtkRange

**`UKOEnemyDataAsset`:** EnemyID, EnemyStat, SkeletalMesh, AnimMontages, EnemyBlendSpace, EnemyAnimationBlueprint, ProjectileStaticMesh, HitParticle, EnemyBT, EnemyMaterial, AIControllerClass — 모두 `TSoftObjectPtr`/`TSoftClassPtr` (비동기 로딩 의도). 현재 `AKOBaseEnemy::SetupEnemy()`는 미구현 stub.

**데이터 에셋 위치:** `Source/Karon/Data/` — `KOComboActionData`, `KOInputConfig`, `KOGrantSet`, `KOEnemyDataAsset`, `KOBossDataAsset`, `KOWeaponDefinition`

---

### GameplayTag 네임스페이스 구조

`KOGameplayTags.h`는 **umbrella 헤더** (직접 편집 금지). 새 태그는 해당 도메인 하위 파일에 추가:

```
AbilitySystem/Tag/
├── KOGameplayTags.h                        ← 유일한 include 포인트
├── Data/KOGameplayTags_Data.h              → Data.Message.*, Data.Attribute.*
├── Event/KOGameplayTags_Event.h            → Event.*
├── Gameplay/KOGameplayTags_Gameplay.h      → Gameplay.*
├── Input/KOGameplayTags_Input.h            → Input.*
├── Item/KOGameplayTags_Item.h              → Item.*
├── Skill/KOGameplayTags_Skill.h            → Skill.*
├── State/KOGameplayTags_State.h            → State.*
└── UI/KOGameplayTags_UI.h                  → UI.*
```

---

### 유틸리티

- `Source/Karon/Debug/KOGASDebugLibrary`: GAS 상태 디버그 출력 (런타임)
- `Source/Karon/Utility/Log/KOLogManager`: 프로젝트 전용 로그 카테고리
- `Source/Karon/Utility/Interface/`: `IKOInteractableInterface` 등 공용 인터페이스

---

## 주요 코딩 규칙

클래스 네이밍 접두어 `KO`: `UKO...`(UObject/Component/Subsystem), `AKO...`(Actor), `FKO...`(Struct), `EKO...`(Enum), `IKO...`(Interface)

- **GameplayTag 추가:** umbrella가 아닌 도메인 하위 파일(`Tag/[Domain]/KOGameplayTags_[Domain].h`)에 추가.
- **새 어트리뷰트:** `UKOAttributeSetBase` 상속 필수. 어트리뷰트 셋은 `AKOPlayerState`에 등록.
- **시스템 간 통신:** 직접 참조 대신 GMRouter 메시지 채널 우선 검토.
- **위젯 BP 연결:** C++ 멤버에 `UPROPERTY(meta=(BindWidget))` 적용, 클래스는 `UCommonUserWidget` 상속.
- **적 AI 파라미터 설정:** `AKOBaseEnemyAIController::SetAI()`로만 블랙보드 초기화. 개별 키 직접 쓰기 금지.
- **새 어빌리티(영웅):** `UKOGameplayAbilityBase` 상속 후 `UKOGrantSet` 데이터 에셋에 InputTag와 함께 등록.
- **새 어빌리티(적):** 영웅과 동일하게 `UKOGrantSet` DataAsset 생성 후 `AbilitySystemComponent`에 설정. 적 타입별 `DA_KO~GrantSet.uasset`에 어빌리티 추가.
- **보스 AI 추가 시:** `AKOBossBase`를 상속하고 `OnBossInitialized`, `OnPhaseChanged`, `OnBossDeath`, `OnGroggyBegin`, `OnGroggyEnd` 가상 함수 오버라이드. 기믹 비율은 `GimmickReadyRatios` 배열로 여러 개 설정 가능.
- **무기 어빌리티:** `UKOGrantSet`에 묶어서 장착 시 `EquipWeapon()`으로, 해제 시 `FKOAbilitySetHandles::RemoveFromASC()`로 일괄 처리.
- **컨베이어 등록/해제:** `UKOConveyorSubsystem::EnqueueAction()`으로 deferred 처리. Tick 도중 직접 등록/해제 금지.
- **데이터 조회:** 모든 DataTable 접근은 `UKOLoadSubsystem`을 경유. DataTable에 직접 접근하지 않음.
