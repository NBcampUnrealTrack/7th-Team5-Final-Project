// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Subsystem/KOEnergyTypes.h"
#include "Subsystem/KOItemPortTypes.h"
#include "KOFactoryProcessorComponent.generated.h"

class AKOBaseBuilding;
struct FKORecipeRow;

UENUM()
enum class EKOFactoryState : uint8
{
    Idle,           // 가공할 레시피 없음 or 입력 부족
    Running,        // 사이클 진행 중
    OutputBlocked   // 출력 버퍼 가득 참
};

UCLASS(ClassGroup = "KO|Factory", meta = (BlueprintSpawnableComponent))
class KARON_API UKOFactoryProcessorComponent : public UActorComponent, public IKOEnergyConsumer, public IKOItemSource, public IKOItemSink
{
    GENERATED_BODY()

public:
    UKOFactoryProcessorComponent();
    
    UPROPERTY(EditAnywhere, Category = "KO|Factory")
    bool bAutoStart = true;

    UPROPERTY(EditAnywhere, Category = "KO|Factory")
    int32 MaxBufferPerItem = 999;
    
    int32 TryInsertItem(FName ItemId, int32 Count);
    int32 TryExtractItem(FName ItemId, int32 Count);

    /** InputBuffer에서 차감해서 회수. 실제 추출된 수량 반환. */
    int32 TryExtractInputItem(FName ItemId, int32 Count);

    /** TryExtractItem 후 인벤토리가 못 받은 잔량을 OutputBuffer에 되돌리는 헬퍼. 캡 검증 없음(직전 추출량 이하 가정). */
    void  RestoreOutputBuffer(FName ItemId, int32 Count);

    /** TryExtractInputItem 후 인벤토리가 못 받은 잔량을 InputBuffer에 되돌리는 헬퍼. */
    void  RestoreInputBuffer(FName ItemId, int32 Count);

    /** 수동 레시피 선택. NAME_None을 넘기면 자동 선택으로 복귀. 즉시 가동 시도. */
    void  SetSelectedRecipe(FName RecipeId);
    FName GetSelectedRecipe() const { return SelectedRecipeId; }
    bool  ManualStart();
    
    float GetProgress() const;

    EKOFactoryState GetState()         const { return State; }
    FName GetActiveRecipeId()          const { return ActiveRecipeId; }
    float GetLastSupplyRatio()         const { return LastSupplyRatio; }
    
    bool HasAnyInputItems() const;
    bool HasAnyOutputItems() const;
    bool CanChangeRecipe() const;

    /** 가동 중 요구 전력(초당). 비가동/레시피 없음이면 0. */
    float GetRequestedPowerPerSecond() const { return GetActiveRecipePowerPerSecond(); }
    /** 직전 틱 실제 소비 전력(초당) = 요구 × 공급률. */
    float GetSuppliedPowerPerSecond()  const { return GetActiveRecipePowerPerSecond() * LastSupplyRatio; }

    const TMap<FName, int32>& GetInputBuffer()  const { return InputBuffer; }
    const TMap<FName, int32>& GetOutputBuffer() const { return OutputBuffer; }

    // IKOEnergyConsumer
    virtual float GetPowerDemand(float DeltaSeconds) const override;
    virtual void  OnPowerSupplied(float SuppliedAmount, float RequestedAmount) override;
    virtual void  GetEnergyOccupiedCells(TArray<FIntPoint>& OutCells) const override;

    // IKOItemSource (출력 버퍼를 벨트로 내보냄)
    virtual bool PeekOutputItem(FKOConveyorItem& OutItem) const override;
    virtual bool PopOutputItem(FKOConveyorItem& OutItem) override;

    // IKOItemSink (벨트가 입력 버퍼로 넣음)
    virtual bool CanAcceptItem(const FKOConveyorItem& Item) const override;
    virtual bool PushItem(const FKOConveyorItem& Item) override;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    // Internal Functions
    FName FindRunnableRecipe() const;
    bool  TryStartCycle();
    void  OnCycleComplete();

    bool  HasInputsFor (const FKORecipeRow& Recipe) const;
    bool  CanFitOutputs(const FKORecipeRow& Recipe) const;
    float GetActiveRecipePowerPerSecond() const;

    void  EvaluateAutoStart();
    void  SetState(EKOFactoryState NewState);
    void  BroadcastStateChanged() const;
    void  BroadcastProcessorChanged() const;

    AKOBaseBuilding* GetOwnerBuilding() const;
    FName GetOwnerFactoryId()           const;

    // Internal State
    EKOFactoryState State = EKOFactoryState::Idle;
    FName  ActiveRecipeId      = NAME_None;
    FName  SelectedRecipeId    = NAME_None;
    float  CurrentCycleSeconds = 0.f;
    float  Progress            = 0.f;   
    float  LastSupplyRatio     = 1.f;  

    UPROPERTY()
    TMap<FName, int32> InputBuffer;

    UPROPERTY()
    TMap<FName, int32> OutputBuffer;
};
