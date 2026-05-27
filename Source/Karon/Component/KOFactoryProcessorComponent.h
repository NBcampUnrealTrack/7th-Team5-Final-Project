// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Subsystem/KOEnergyTypes.h"
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
class KARON_API UKOFactoryProcessorComponent : public UActorComponent, public IKOEnergyConsumer
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
    bool  ManualStart();
    
    float GetProgress() const;

    EKOFactoryState GetState()         const { return State; }
    FName GetActiveRecipeId()          const { return ActiveRecipeId; }
    float GetLastSupplyRatio()         const { return LastSupplyRatio; }

    const TMap<FName, int32>& GetInputBuffer()  const { return InputBuffer; }
    const TMap<FName, int32>& GetOutputBuffer() const { return OutputBuffer; }

    // IKOEnergyConsumer 
    virtual float GetPowerDemand(float DeltaSeconds) const override;
    virtual void  OnPowerSupplied(float SuppliedAmount, float RequestedAmount) override;

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

    AKOBaseBuilding* GetOwnerBuilding() const;
    FName GetOwnerFactoryId()           const;

    // Internal State
    EKOFactoryState State = EKOFactoryState::Idle;
    FName  ActiveRecipeId      = NAME_None;
    float  CurrentCycleSeconds = 0.f;
    float  Progress            = 0.f;   
    float  LastSupplyRatio     = 1.f;  

    UPROPERTY()
    TMap<FName, int32> InputBuffer;

    UPROPERTY()
    TMap<FName, int32> OutputBuffer;
};
