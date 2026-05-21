// Copyright Karon Team 5. All Rights Reserved.
#include "KOUISubsystem.h"

#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Messaging/KOMessageTypes.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "CommonActivatableWidget.h"

#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"

DEFINE_LOG_CATEGORY(LogKOUI);

UKOUISubsystem* UKOUISubsystem::Get(const APlayerController* PlayerController)
{
    if (!PlayerController)
    {
        return nullptr;
    }
    return GetForLocalPlayer(PlayerController->GetLocalPlayer());
}

UKOUISubsystem* UKOUISubsystem::GetForLocalPlayer(const ULocalPlayer* LocalPlayer)
{
    return LocalPlayer ? LocalPlayer->GetSubsystem<UKOUISubsystem>() : nullptr;
}

void UKOUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UGameInstance* GI = GetLocalPlayer()->GetGameInstance();
    if (!GI)
    {
        return;
    }
    
    if (UGMRouterSubsystem* GMS = GI->GetSubsystem<UGMRouterSubsystem>())
    {
        PushLayerCallback.BindDynamic(this, &UKOUISubsystem::OnPushLayerRequestReceived);
        PushLayerHandle = GMS->Subscribe(KOGameplayTags::Data_Message_UI_PushLayerRequest, PushLayerCallback);
    }
    else
    {
        UE_LOG(LogKOUI, Warning, TEXT("KOUISubsystem: UGMRouterSubsystem를 찾을 수 없어 PushLayerRequest 구독을 건너뜁니다."));
    }
}

void UKOUISubsystem::Deinitialize()
{
    if (PushLayerHandle.IsValid())
    {
        if (ULocalPlayer* LP = GetLocalPlayer())
        {
            if (UGMRouterSubsystem* GMS = LP->GetGameInstance()->GetSubsystem<UGMRouterSubsystem>())
            {
                GMS->Unsubscribe(PushLayerHandle);
            }
        }
        PushLayerHandle = FGameplayMessageHandle();
    }

    PushLayerCallback.Clear();
    Layers.Empty();

    Super::Deinitialize();
}

// ─── Layout Registration ──────────────────────────────────────────────────────
void UKOUISubsystem::RegisterPrimaryLayout(FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase* LayerContainer)
{
    if (!LayerTag.IsValid())
    {
        UE_LOG(LogKOUI, Warning, TEXT("KOUISubsystem::RegisterPrimaryLayout: 유효하지 않은 LayerTag입니다."));
        return;
    }

    if (!IsValid(LayerContainer))
    {
        UE_LOG(LogKOUI, Warning, TEXT("KOUISubsystem::RegisterPrimaryLayout: LayerContainer가 유효하지 않습니다. Tag=%s"),
            *LayerTag.ToString());
        return;
    }

    Layers.Add(LayerTag, LayerContainer);
    UE_LOG(LogKOUI, Log, TEXT("KOUISubsystem: 레이어 등록 완료 [%s]"), *LayerTag.ToString());
}

// ─── Widget Stack API ─────────────────────────────────────────────────────────
UCommonActivatableWidget* UKOUISubsystem::PushLayer(FGameplayTag LayerTag, TSubclassOf<UCommonActivatableWidget> WidgetClass)
{
    if (!LayerTag.IsValid() || !WidgetClass)
    {
        UE_LOG(LogKOUI, Warning, TEXT("KOUISubsystem::PushLayer: LayerTag 또는 WidgetClass가 유효하지 않습니다."));
        return nullptr;
    }

    TObjectPtr<UCommonActivatableWidgetContainerBase>* ContainerPtr = Layers.Find(LayerTag);
    if (!ContainerPtr || !IsValid(*ContainerPtr))
    {
        UE_LOG(LogKOUI, Warning,
            TEXT("KOUISubsystem::PushLayer: 레이어 [%s]가 등록되어 있지 않습니다. RegisterPrimaryLayout()을 먼저 호출하세요."),
            *LayerTag.ToString());
        return nullptr;
    }

    UCommonActivatableWidget* NewWidget = (*ContainerPtr)->AddWidget<UCommonActivatableWidget>(WidgetClass);
    return NewWidget;
}

void UKOUISubsystem::PopLayer(UCommonActivatableWidget* Widget)
{
    if (!IsValid(Widget))
    {
        UE_LOG(LogKOUI, Warning, TEXT("KOUISubsystem::PopLayer: Widget이 유효하지 않습니다."));
        return;
    }

    // CommonActivatableWidget을 비활성화하면 소속 Stack/Queue 컨테이너가 자동으로 제거 처리한다.
    Widget->DeactivateWidget();
}

// ─── GMS 콜백 ─────────────────────────────────────────────────────────────────
void UKOUISubsystem::OnPushLayerRequestReceived(FGameplayTag Channel, const FInstancedStruct& Payload)
{
    const FKOUIPushLayerRequest* Request = Payload.GetPtr<FKOUIPushLayerRequest>();
    if (!Request)
    {
        UE_LOG(LogKOUI, Warning, TEXT("KOUISubsystem: PushLayerRequest 페이로드 파싱 실패."));
        return;
    }

    PushLayer(Request->LayerTag, Request->WidgetClass);
}
