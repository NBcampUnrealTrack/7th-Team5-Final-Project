// Copyright Karon Team 5. All Rights Reserved.
#include "KOActivatableWidget.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"

// ─── UCommonActivatableWidget ─────────────────────────────────────────────────

void UKOActivatableWidget::NativeOnActivated()
{
    Super::NativeOnActivated();
    // 파생 클래스는 Super 호출 이후 GMS 구독을 등록한다.
    // 예:
    //   FGameplayMessageCallback Callback;
    //   Callback.BindDynamic(this, &UMyWidget::OnReceived);
    //   SubscribeToGMS(KOGameplayTags::Message_Inventory_Changed, Callback);
}

void UKOActivatableWidget::NativeOnDeactivated()
{
    // 파생 클래스는 Super 호출 이전에 GMS 구독을 해제한다.
    // 예:
    //   UnsubscribeFromGMS(KOGameplayTags::Message_Inventory_Changed, Callback);
    //   Callback.Clear();
    Super::NativeOnDeactivated();
}

// ─── GMS 헬퍼 ─────────────────────────────────────────────────────────────────

void UKOActivatableWidget::SubscribeToGMS(FGameplayTag Channel, const FGameplayMessageCallback& Callback)
{
    if (!Channel.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("KOActivatableWidget::SubscribeToGMS: 유효하지 않은 Channel 태그입니다."));
        return;
    }

    if (UKHS_GMRouterManager* GMS = GetGMSSubsystem())
    {
        GMS->SubscribeToMessage(Channel, Callback);
    }
}

void UKOActivatableWidget::UnsubscribeFromGMS(FGameplayTag Channel, const FGameplayMessageCallback& Callback)
{
    if (!Channel.IsValid())
    {
        return;
    }

    if (UKHS_GMRouterManager* GMS = GetGMSSubsystem())
    {
        GMS->Unsubscribe(Channel, Callback);
    }
}

void UKOActivatableWidget::BroadcastGMS(FGameplayTag Channel, const FInstancedStruct& Payload)
{
    if (!Channel.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("KOActivatableWidget::BroadcastGMS: 유효하지 않은 Channel 태그입니다."));
        return;
    }

    if (UKHS_GMRouterManager* GMS = GetGMSSubsystem())
    {
        GMS->BroadcastMessage(Channel, Payload);
    }
}

// ─── Private ──────────────────────────────────────────────────────────────────

UKHS_GMRouterManager* UKOActivatableWidget::GetGMSSubsystem() const
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

    return GI->GetSubsystem<UKHS_GMRouterManager>();
}
