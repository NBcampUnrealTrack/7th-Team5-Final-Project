// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Subsystem/KOEnergyTypes.h"
#include "KOEnergyProducerComponent.generated.h"

UCLASS(ClassGroup = "KO|Factory", meta = (BlueprintSpawnableComponent))
class KARON_API UKOEnergyProducerComponent : public UActorComponent, public IKOEnergyProducer
{
    GENERATED_BODY()

public:
    UKOEnergyProducerComponent();
    /**
     * 받아들이는 연료 카테고리 태그. FKOItemRow::Categories 에 이 태그가 포함되면 연료로 인정.
     * 기본값: "Item.Category.EnergyResource".
     */
    UPROPERTY(EditAnywhere, Category = "KO|Energy")
    FGameplayTag FuelCategoryTag;

    /** 연료 1개 소모 시 생산되는 총 에너지량 */
    UPROPERTY(EditAnywhere, Category = "KO|Energy")
    float PowerPerFuelUnit = 100.f;

    /** 초당 태울 수 있는 최대 연료 개수 */
    UPROPERTY(EditAnywhere, Category = "KO|Energy")
    float BurnRatePerSecond = 1.f;

    /** 연료 버퍼 최대 누적량 */
    UPROPERTY(EditAnywhere, Category = "KO|Energy")
    int32 MaxFuelBuffer = 999;
    
    /** 연료 적재. 반환값: 받지 못한 잔여 수량. */
    int32 TryInsertFuel(FName ItemId, int32 Count);

    int32 GetFuelCount() const { return FuelInBuffer; }
    FName GetFuelItemId() const { return FuelItemId; }

    // IKOEnergyProducer 
    virtual float GetPowerOutput(float DeltaSeconds) const override;
    virtual void  OnPowerAccepted(float Amount) override;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;

private:
    /** 정수 단위 연료 보유량. */
    int32 FuelInBuffer = 0;

    /** 소수 단위 연료 보유량 */
    float FuelDebt = 0.f;

    /** 현재 적재된 연료 아이템 ID (UI 표시용). 비었을 때 NAME_None. */
    FName FuelItemId = NAME_None;
};
