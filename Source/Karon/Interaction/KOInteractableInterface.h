// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "KOInteractableInterface.generated.h"

class AActor;

UINTERFACE(MinimalAPI, BlueprintType, Blueprintable)
class UKOInteractableInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 * IKOInteractableInterface
 * 플레이어가 상호작용 가능한 액터가 구현하는 인터페이스.
 * C++/BP 양쪽에서 구현 가능. 호출은 항상 Execute_XXX(Actor, ...) 형태를 사용.
 */
class KARON_API IKOInteractableInterface
{
    GENERATED_BODY()

public:
    /** 지금 이 액터와 상호작용할 수 있는가 */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    bool CanInteract(AActor* Interactor) const;
    virtual bool CanInteract_Implementation(AActor* Interactor) const { return true; }

    /** 상호작용 실행. Interactor는 보통 플레이어 Pawn 또는 Controller */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    void OnInteract(AActor* Interactor);
    virtual void OnInteract_Implementation(AActor* Interactor) {}

    /** UI에 표시할 안내문 (예: "F: 보일러 열기") */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    FText GetInteractionPrompt() const;
    virtual FText GetInteractionPrompt_Implementation() const { return FText::GetEmpty(); }
};
