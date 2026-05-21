// Copyright Karon Team 5. All Rights Reserved.
#include "Messaging/KOGMSInterface.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "StructUtils/InstancedStruct.h"

DEFINE_LOG_CATEGORY_STATIC(LogKOGMS, Log, All);

FGameplayMessageHandle IKOGMSInterface::Subscribe(FGameplayTag Channel, const FGameplayMessageCallback& Callback)
{
    if (!Channel.IsValid())
    {
        UE_LOG(LogKOGMS, Warning, TEXT("IKOGMSInterface::Subscribe: 유효하지 않은 Channel 태그입니다."));
        return FGameplayMessageHandle();
    }

    if (UGMRouterSubsystem* GMS = GetGMS())
    {
        return GMS->Subscribe(Channel, Callback);
    }

    return FGameplayMessageHandle();
}

void IKOGMSInterface::Unsubscribe(const FGameplayMessageHandle& Handle)
{
    if (!Handle.IsValid())
    {
        return;
    }

    if (UGMRouterSubsystem* GMS = GetGMS())
    {
        GMS->Unsubscribe(Handle);
    }
}

void IKOGMSInterface::Broadcast(FGameplayTag Channel, const FInstancedStruct& Payload)
{
    if (!Channel.IsValid())
    {
        UE_LOG(LogKOGMS, Warning, TEXT("IKOGMSInterface::Broadcast: 유효하지 않은 Channel 태그입니다."));
        return;
    }

    if (UGMRouterSubsystem* GMS = GetGMS())
    {
        GMS->BroadcastMessage(Channel, Payload);
    }
}

UGMRouterSubsystem* IKOGMSInterface::GetGMS() const
{
    const UObject* AsObject = Cast<UObject>(this);
    if (!AsObject)
    {
        return nullptr;
    }

    const UWorld* World = AsObject->GetWorld();
    if (!World)
    {
        return nullptr;
    }

    const UGameInstance* GI = World->GetGameInstance();
    if (!GI)
    {
        return nullptr;
    }

    return GI->GetSubsystem<UGMRouterSubsystem>();
}
