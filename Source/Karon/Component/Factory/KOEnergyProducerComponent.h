// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Subsystem/KOEnergyTypes.h"
#include "Subsystem/KOItemPortTypes.h"
#include "KOEnergyProducerComponent.generated.h"

UCLASS(ClassGroup = "KO|Factory", meta = (BlueprintSpawnableComponent))
class KARON_API UKOEnergyProducerComponent : public UActorComponent, public IKOEnergyProducer, public IKOItemSink
{
    GENERATED_BODY()

public:
    UKOEnergyProducerComponent();
    /** 연료 버퍼 최대 누적량 */
    UPROPERTY(EditAnywhere, Category = "KO|Energy")
    int32 MaxFuelBuffer = 999;
    
    /** 연료 적재. 반환값: 받지 못한 잔여 수량. */
    int32 TryInsertFuel(FName ItemId, int32 Count);

    /** 연료 회수. 현재 적재된 FuelItemId 기준으로 차감. 반환값: 실제 추출 수량. */
    int32 TryExtractFuel(int32 Count);

    /** TryExtractFuel 후 인벤토리가 못 받은 잔량을 되돌리는 헬퍼. 캡 검증 없음. */
    void  RestoreFuelBuffer(FName ItemId, int32 Count);

    int32 GetFuelCount() const { return FuelInBuffer; }
    FName GetFuelItemId() const { return FuelItemId; }

    /** 현재 연료 1단위의 소모 진행도(0~1). 연료 없으면 0. 전력이 실제 소비될 때만 진행. UI ProgressBar 용. */
    float GetBurnProgress() const { return bHasActiveFuel ? FMath::Clamp(FuelDebt, 0.f, 1.f) : 0.f; }
    FName GetRecipeId() const { return RecipeId; }
    FName GetAcceptedFuelItemId() const { return AcceptedFuelItemId; }
    float GetPowerPerFuelUnit() const { return PowerPerFuelUnit; }
    float GetBurnRatePerSecond() const { return BurnRatePerSecond; }

    /** 연료 1단위를 모두 태우는 데 걸리는 시간(초). 레시피 CycleSeconds 와 동일. 연료 정보 없으면 0. UI 표시용. */
    float GetCycleSeconds() const { return (BurnRatePerSecond > 0.f) ? (1.f / BurnRatePerSecond) : 0.f; }

    /** 직전 틱 이 발전기가 실제로 공급한 에너지의 초당 환산(수요/연료 반영 후). */
    float GetCurrentOutputPerSecond() const { return LastOutputRate; }

    /** 연료가 있을 때 낼 수 있는 최대 초당 출력(BurnRate × PowerPerFuel). 연료 없으면 0. */
    float GetMaxOutputPerSecond() const
    {
        return bHasActiveFuel ? BurnRatePerSecond * PowerPerFuelUnit : 0.f;
    }

    // IKOEnergyProducer
    virtual float GetPowerOutput(float DeltaSeconds) const override;
    virtual void  OnPowerAccepted(float Amount) override;
    virtual void  GetEnergyCoverageCells(TArray<FIntPoint>& OutCells) const override;

    // IKOItemSink (벨트가 연료 입구로 넣음)
    virtual bool CanAcceptItem(const FKOConveyorItem& Item) const override;
    virtual bool PushItem(const FKOConveyorItem& Item) override;

    // 세이브 로드
    float GetFuelDebtForSave() const { return FuelDebt; }
    bool HasActiveFuelForSave() const { return bHasActiveFuel; }
    
    void LoadFuelFromSave(FName InFuelItemId, int32 InFuelCount, float InFuelDebt, bool bInHasActiveFuel);

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;

public:
    void InitializeFromRecipe();

private:
    void BroadcastFuelChanged() const;
    void StartNextFuelIfNeeded();
    
    FName RecipeId = NAME_None;
    float PowerPerFuelUnit = 0.f;
    float BurnRatePerSecond = 0.f;
    FName AcceptedFuelItemId = NAME_None;

    /** 대기 중인 연료 개수. */
    int32 FuelInBuffer = 0;

    /** 현재 타는 중인 연료의 연소 진행도 */
    float FuelDebt = 0.f;
    
    /** 현재 타는 중인 연료가 있는지 */
    bool bHasActiveFuel = false;

    /** 직전 틱 실제 공급 에너지의 초당 환산(UI 표시용). 미공급 시 0. */
    float LastOutputRate = 0.f;

    /** 현재 적재된 연료 아이템 ID (UI 표시용). 비었을 때 NAME_None. */
    FName FuelItemId = NAME_None;
};
