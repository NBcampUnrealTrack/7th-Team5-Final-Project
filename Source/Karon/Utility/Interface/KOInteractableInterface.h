// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "KOInteractableInterface.generated.h"

class AActor;

UINTERFACE(MinimalAPI)
class UKOInteractableInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 * IKOInteractableInterface
 * 플레이어가 상호작용 가능한 액터가 구현하는 인터페이스.
 * 순수 C++ 인터페이스. 구현체는 virtual 함수를 override하고, 호출은 직접 한다.
 */
class KARON_API IKOInteractableInterface
{
    GENERATED_BODY()

public:
    /** 지금 이 액터와 상호작용할 수 있는가 */
    virtual bool CanInteract(AActor* Interactor) const { return true; }

    /** 상호작용 실행. Interactor는 보통 플레이어 Pawn 또는 Controller */
    virtual void OnInteract(AActor* Interactor) {}

    /** UI에 표시할 안내문 (예: "F: 보일러 열기") */
    virtual FText GetInteractionPrompt() const { return FText::GetEmpty(); }
};
