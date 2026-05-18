// Copyright Karon Team 5. All Rights Reserved.
#include "KOActivatableWidget.h"
#include "KOUISubsystem.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"

// ─── UCommonActivatableWidget ─────────────────────────────────────────────────

void UKOActivatableWidget::NativeOnActivated()
{
    Super::NativeOnActivated();
}

void UKOActivatableWidget::NativeOnDeactivated()
{
    Super::NativeOnDeactivated();
}

// ─── GMS 헬퍼 ─────────────────────────────────────────────────────────────────

FGameplayMessageHandle UKOActivatableWidget::SubscribeToGMS(FGameplayTag Channel, const FGameplayMessageCallback& Callback)
{
    if (!Channel.IsValid())
    {
        UE_LOG(LogKOUI, Warning, TEXT("KOActivatableWidget::SubscribeToGMS: 유효하지 않은 Channel 태그입니다."));
        return FGameplayMessageHandle();
    }

    if (UGMRouterSubsystem* GMS = GetGMSSubsystem())
    {
        return GMS->Subscribe(Channel, Callback);
    }

    return FGameplayMessageHandle();
}

void UKOActivatableWidget::UnsubscribeFromGMS(const FGameplayMessageHandle& Handle)
{
    if (!Handle.IsValid())
    {
        return;
    }

    if (UGMRouterSubsystem* GMS = GetGMSSubsystem())
    {
        GMS->Unsubscribe(Handle);
    }
}

void UKOActivatableWidget::BroadcastGMS(FGameplayTag Channel, const FInstancedStruct& Payload)
{
    if (!Channel.IsValid())
    {
        UE_LOG(LogKOUI, Warning, TEXT("KOActivatableWidget::BroadcastGMS: 유효하지 않은 Channel 태그입니다."));
        return;
    }

    if (UGMRouterSubsystem* GMS = GetGMSSubsystem())
    {
        GMS->BroadcastMessage(Channel, Payload);
    }
}

// ─── Private ──────────────────────────────────────────────────────────────────

UGMRouterSubsystem* UKOActivatableWidget::GetGMSSubsystem() const
{
    const UWorld* World = GetWorld();
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
