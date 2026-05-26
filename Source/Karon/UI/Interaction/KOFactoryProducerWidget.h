// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UI/KOActivatableWidget.h"
#include "KOFactoryProducerWidget.generated.h"

class AKOBaseBuilding;
class UKOEnergyProducerComponent;

/**
 * UKOFactoryProducerWidget
 * Producer(에너지 생산 설비) 상호작용 UI.
 * 현재 Producer 전용 상태 변경 메시지 채널이 없어 초기 1회 풀(pull)만 수행.
 * 연료 변동 메시지 도입 시 NativeOnActivated/Deactivated에서 구독을 추가하면 됨.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class KARON_API UKOFactoryProducerWidget : public UKOActivatableWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category = "KO|UI|Interaction")
    AKOBaseBuilding* GetTargetBuilding() const { return TargetBuilding.Get(); }

    UFUNCTION(BlueprintPure, Category = "KO|UI|Interaction")
    UKOEnergyProducerComponent* GetProducer() const { return Producer.Get(); }

protected:
    virtual void NativeOnActivated() override;
    virtual void NativeOnDeactivated() override;

    UFUNCTION(BlueprintImplementableEvent, Category = "KO|UI|Interaction")
    void BP_OnProducerRefreshed();

private:
    TWeakObjectPtr<AKOBaseBuilding> TargetBuilding;
    TWeakObjectPtr<UKOEnergyProducerComponent> Producer;
};
