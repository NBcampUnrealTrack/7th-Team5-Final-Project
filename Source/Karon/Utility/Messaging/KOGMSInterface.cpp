// Copyright Karon Team 5. All Rights Reserved.
#include "KOGMSInterface.h"

#include "Engine/World.h"
#include "StructUtils/InstancedStruct.h"

DEFINE_LOG_CATEGORY_STATIC(LogKOGMS, Log, All);

namespace
{
    const UWorld* GetWorldFromInterface(const IKOGMSInterface* Self)
    {
        const UObject* AsObject = Cast<UObject>(Self);
        return AsObject ? AsObject->GetWorld() : nullptr;
    }
}

FGameplayMessageHandle IKOGMSInterface::Subscribe(FGameplayTag Channel, const FGameplayMessageCallback& Callback)
{
    if (!Channel.IsValid())
    {
        UE_LOG(LogKOGMS, Warning, TEXT("IKOGMSInterface::Subscribe: 유효하지 않은 Channel 태그입니다."));
        return FGameplayMessageHandle();
    }

    return UGMRouterSubsystem::Subscribe(GetWorldFromInterface(this), Channel, Callback);
}

void IKOGMSInterface::Unsubscribe(FGameplayMessageHandle& Handle)
{
    Handle.Unsubscribe();
}

void IKOGMSInterface::Broadcast(FGameplayTag Channel, const FInstancedStruct& Payload)
{
    if (!Channel.IsValid())
    {
        UE_LOG(LogKOGMS, Warning, TEXT("IKOGMSInterface::Broadcast: 유효하지 않은 Channel 태그입니다."));
        return;
    }

    UGMRouterSubsystem::BroadcastMessage(GetWorldFromInterface(this), Channel, Payload);
}
