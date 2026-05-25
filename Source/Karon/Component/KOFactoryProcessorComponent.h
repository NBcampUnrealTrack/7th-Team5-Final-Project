// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "KOFactoryProcessorComponent.generated.h"

class AKOBaseBuilding;
struct FKORecipeRow;

UENUM()
enum class EKOFactoryState : uint8
{
    Idle,           // 가공할 레시피 없음 / 입력 부족
    Running,        // 사이클 진행 중
    OutputBlocked   // 사이클 끝났으나 출력 버퍼 가득
};

/**
 * UKOFactoryProcessorComponent
 * AKOBaseBuilding(공장)에 부착되어 "입력 소비 → 사이클 대기 → 출력 적재"를 수행한다.
 *
 * - FactoryId는 Owner의 GetFactoryId()에서 얻는다.
 * - LoadSubsystem을 통해 본인 FactoryId가 AllowedFactoryIds에 포함된 Recipe를 찾는다.
 * - 입력 충분 + 출력 여유면 사이클 시작, 종료 시 입력 차감 / 출력 적재.
 * - 상태 변경 시 GMRouter로 Data_Message_Factory_StateChanged 브로드캐스트.
 *
 * 모드:
 *  - bAutoStart=true  : Idle 진입 시 자동으로 가능한 레시피 탐색·시작 (기본값)
 *  - bAutoStart=false : ManualStart() 호출 시에만 사이클 시작 (예: WeaponWorkbench)
 */
UCLASS(ClassGroup = "KO|Factory")
class KARON_API UKOFactoryProcessorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UKOFactoryProcessorComponent();

    // ─── 설정 ─────────────────────────────────────────────────────────────────
    /** Idle 진입 시 자동으로 가능한 레시피를 탐색·시작할지 여부. */
    UPROPERTY(EditAnywhere, Category = "KO|Factory")
    bool bAutoStart = true;

    /** 내부 입력·출력 버퍼의 아이템 종류당 최대 누적량. */
    UPROPERTY(EditAnywhere, Category = "KO|Factory")
    int32 MaxBufferPerItem = 999;

    // ─── 외부 API ─────────────────────────────────────────────────────────────
    /** 입력 버퍼에 아이템을 넣는다. 반환값: 받지 못한 잔여 수량. */
    int32 TryInsertItem(FName ItemId, int32 Count);

    /** 출력 버퍼에서 아이템을 빼간다. 반환값: 실제로 빼간 수량. */
    int32 TryExtractItem(FName ItemId, int32 Count);

    /** bAutoStart=false 일 때 외부에서 사이클 시작을 트리거. 성공 시 true. */
    bool ManualStart();

    /** 현재 사이클 진행률 [0, 1]. Running이 아니면 0. */
    float GetProgress01() const;

    EKOFactoryState GetState() const { return State; }
    FName GetActiveRecipeId()  const { return ActiveRecipeId; }

    const TMap<FName, int32>& GetInputBuffer()  const { return InputBuffer; }
    const TMap<FName, int32>& GetOutputBuffer() const { return OutputBuffer; }

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;

private:
    // ─── 내부 동작 ────────────────────────────────────────────────────────────
    /** 현재 상황에서 시작 가능한 레시피를 찾는다 (없으면 NAME_None). */
    FName FindRunnableRecipe() const;

    /** 자동/수동 진입 공통: 시작 가능한 레시피가 있으면 사이클 개시. */
    bool TryStartCycle();

    void OnCycleComplete();

    bool HasInputsFor (const FKORecipeRow& Recipe) const;
    bool CanFitOutputs(const FKORecipeRow& Recipe) const;

    /** Idle/OutputBlocked → 가능하면 자동 시작 시도 (bAutoStart일 때). */
    void EvaluateAutoStart();

    void SetState(EKOFactoryState NewState);
    void BroadcastStateChanged() const;

    AKOBaseBuilding* GetOwnerBuilding() const;
    FName GetOwnerFactoryId()           const;

    // ─── 상태 ─────────────────────────────────────────────────────────────────
    EKOFactoryState State = EKOFactoryState::Idle;
    FName  ActiveRecipeId = NAME_None;
    float  CurrentCycleSeconds = 0.f;
    double CycleStartSeconds   = 0.0;
    FTimerHandle CycleTimer;

    UPROPERTY()
    TMap<FName, int32> InputBuffer;

    UPROPERTY()
    TMap<FName, int32> OutputBuffer;
};
