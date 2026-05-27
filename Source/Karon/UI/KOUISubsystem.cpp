// Copyright Karon Team 5. All Rights Reserved.
#include "KOUISubsystem.h"

#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Messaging/KOMessageTypes.h"
#include "UI/KOUISettings.h"
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
    ResolvedClassCache.Empty();
    ActiveWidgetsByTag.Empty();

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

UCommonActivatableWidget* UKOUISubsystem::FindActiveWidget(FGameplayTag WidgetTag) const
{
    if (const TWeakObjectPtr<UCommonActivatableWidget>* WeakPtr = ActiveWidgetsByTag.Find(WidgetTag))
    {
        return WeakPtr->Get();
    }
    return nullptr;
}

bool UKOUISubsystem::ToggleWidget(FGameplayTag WidgetTag)
{
    if (UCommonActivatableWidget* Existing = FindActiveWidget(WidgetTag))
    {
        PopLayer(Existing);
        return false;
    }

    return PushWidget(WidgetTag) != nullptr;
}

UCommonActivatableWidget* UKOUISubsystem::PushWidget(FGameplayTag WidgetTag)
{
    if (!WidgetTag.IsValid())
    {
        UE_LOG(LogKOUI, Warning, TEXT("KOUISubsystem::PushWidget: WidgetTag가 유효하지 않습니다."));
        return nullptr;
    }
    
    // 중복 push 방지(에: 건설 모드, 인벤토리 -> 퀵슬롯)
    if (UCommonActivatableWidget* Existing = FindActiveWidget(WidgetTag))
    {
        return Existing;
    }

    const UKOUISettings* Settings = UKOUISettings::Get();
    if (!Settings)
    {
        UE_LOG(LogKOUI, Warning, TEXT("KOUISubsystem::PushWidget: KOUISettings 접근 실패."));
        return nullptr;
    }

    const FKOUIWidgetEntry* Entry = Settings->WidgetMap.Find(WidgetTag);
    if (!Entry || !Entry->LayerTag.IsValid())
    {
        UE_LOG(LogKOUI, Warning,
            TEXT("KOUISubsystem::PushWidget: WidgetMap에 매핑이 없거나 LayerTag가 유효하지 않습니다. Tag=%s"),
            *WidgetTag.ToString());
        return nullptr;
    }

    // 캐시 우선 조회 후, 없으면 Soft 참조를 동기 로드하고 캐시.
    TSubclassOf<UCommonActivatableWidget> Class = ResolvedClassCache.FindRef(WidgetTag);
    if (!Class)
    {
        if (Entry->WidgetClass.IsNull())
        {
            UE_LOG(LogKOUI, Warning,
                TEXT("KOUISubsystem::PushWidget: WidgetClass 가 비어 있습니다. Tag=%s"),
                *WidgetTag.ToString());
            return nullptr;
        }

        Class = Entry->WidgetClass.LoadSynchronous();
        if (!Class)
        {
            UE_LOG(LogKOUI, Warning,
                TEXT("KOUISubsystem::PushWidget: WidgetClass 로드 실패. Tag=%s Path=%s"),
                *WidgetTag.ToString(), *Entry->WidgetClass.ToString());
            return nullptr;
        }

        ResolvedClassCache.Add(WidgetTag, Class);
    }

    UCommonActivatableWidget* NewWidget = PushLayer(Entry->LayerTag, Class);
    if (NewWidget)
    {
        ActiveWidgetsByTag.Add(WidgetTag, NewWidget);

        // 외부 경로(ESC/닫기버튼 등)로 Deactivate되어도 맵에서 자동 제거.
        NewWidget->OnDeactivated().AddWeakLambda(this, [this, WidgetTag]()
        {
            ActiveWidgetsByTag.Remove(WidgetTag);
        });
    }
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

    PushWidget(Request->WidgetTag);
}
