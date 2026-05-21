// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "GMRouterSubsystem.h"
#include "KOGMSInterface.generated.h"

struct FInstancedStruct;

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UKOGMSInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 * IKOGMSInterface
 * GMRouter에 대한 Subscribe / Unsubscribe / Broadcast 헬퍼를 제공하는 인터페이스
 * 
 * 사용패턴:
 *   NativeOnActivated:   Callback.BindDynamic(...); Handle = Subscribe(Channel, Callback);
 *   NativeOnDeactivated: Unsubscribe(Handle);
 */
class KARON_API IKOGMSInterface
{
    GENERATED_BODY()

public:
    virtual FGameplayMessageHandle Subscribe(FGameplayTag Channel, const FGameplayMessageCallback& Callback);
    virtual void Unsubscribe(const FGameplayMessageHandle& Handle);
    virtual void Broadcast(FGameplayTag Channel, const FInstancedStruct& Payload);

protected:
    virtual UGMRouterSubsystem* GetGMS() const;
};
