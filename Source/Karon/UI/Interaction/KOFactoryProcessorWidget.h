// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UI/KOActivatableWidget.h"
#include "GMRouterSubsystem.h"
#include "KOFactoryProcessorWidget.generated.h"

class AKOBaseBuilding;
class UKOFactoryProcessorComponent;

/**
 * UKOFactoryProcessorWidget
 * Processor 설비 상호작용 시 표시되는 UI.
 * Activate 시 PC의 InteractionComponent에서 현재 타깃 건물을 가져와
 * UKOFactoryProcessorComponent를 캐시하고, Data.Message.Factory.StateChanged 채널을
 * 구독하여 자기 FactoryId 매칭 메시지만 BP로 전달한다.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class KARON_API UKOFactoryProcessorWidget : public UKOActivatableWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category = "KO|UI|Interaction")
    AKOBaseBuilding* GetTargetBuilding() const { return TargetBuilding.Get(); }

    UFUNCTION(BlueprintPure, Category = "KO|UI|Interaction")
    UKOFactoryProcessorComponent* GetProcessor() const { return Processor.Get(); }

protected:
    virtual void NativeOnActivated() override;
    virtual void NativeOnDeactivated() override;

    /** 초기 1회 + StateChanged 수신 시마다 호출. BP에서 위젯 갱신 구현. */
    UFUNCTION(BlueprintImplementableEvent, Category = "KO|UI|Interaction")
    void BP_OnProcessorRefreshed();

private:
    UFUNCTION()
    void OnFactoryStateChangedGMS(FGameplayTag Channel, const FInstancedStruct& Payload);

    TWeakObjectPtr<AKOBaseBuilding> TargetBuilding;
    TWeakObjectPtr<UKOFactoryProcessorComponent> Processor;

    FGameplayMessageCallback FactoryStateCallback;
    FGameplayMessageHandle   FactoryStateHandle;
};
