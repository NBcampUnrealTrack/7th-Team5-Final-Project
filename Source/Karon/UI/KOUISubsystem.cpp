// Copyright Karon Team 5. All Rights Reserved.
#include "UI/KOUISubsystem.h"

#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Messaging/KOMessageTypes.h"
#include "UI/KOUISettings.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "CommonActivatableWidget.h"

#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

DEFINE_LOG_CATEGORY(LogKOUI);

UKOUISubsystem* UKOUISubsystem::Get(const UObject* WorldContextObject)
{
    const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
    if (!World)
    {
        return nullptr;
    }

    const UGameInstance* GI = World->GetGameInstance();
    ULocalPlayer* LocalPlayer = GI ? GI->GetFirstGamePlayer() : nullptr;
    return LocalPlayer ? LocalPlayer->GetSubsystem<UKOUISubsystem>() : nullptr;
}

UGMRouterSubsystem* UKOUISubsystem::GetRouter() const
{
    const ULocalPlayer* LP = GetLocalPlayer();
    UGameInstance* GI = LP ? LP->GetGameInstance() : nullptr;
    return GI ? GI->GetSubsystem<UGMRouterSubsystem>() : nullptr;
}

// ─── 정적 요청 헬퍼 (GMS 일원화 진입점) ───────────────────────────────────────
void UKOUISubsystem::RequestOpenWidget(const UObject* WorldContextObject, FGameplayTag WidgetTag)
{
    if (!WidgetTag.IsValid())
    {
        return;
    }

    const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
    if (!World)
    {
        UE_LOG(LogKOUI, Warning, TEXT("RequestOpenWidget: World 컨텍스트를 확인할 수 없습니다."));
        return;
    }

    FKOUIWidgetRequest Request;
    Request.WidgetTag = WidgetTag;
    UGMRouterSubsystem::BroadcastMessage(World, KOGameplayTags::Data_Message_UI_OpenWidget, FInstancedStruct::Make(Request));
}

void UKOUISubsystem::RequestCloseWidget(const UObject* WorldContextObject, FGameplayTag WidgetTag)
{
    if (!WidgetTag.IsValid())
    {
        return;
    }

    const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
    if (!World)
    {
        UE_LOG(LogKOUI, Warning, TEXT("RequestCloseWidget: World 컨텍스트를 확인할 수 없습니다."));
        return;
    }

    FKOUIWidgetRequest Request;
    Request.WidgetTag = WidgetTag;
    UGMRouterSubsystem::BroadcastMessage(World, KOGameplayTags::Data_Message_UI_CloseWidget, FInstancedStruct::Make(Request));
}

void UKOUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    if (UGMRouterSubsystem* GMS = GetRouter())
    {
        OpenWidgetCallback.BindDynamic(this, &UKOUISubsystem::OnOpenWidgetRequest);
        OpenWidgetHandle = GMS->Subscribe(KOGameplayTags::Data_Message_UI_OpenWidget, OpenWidgetCallback);

        CloseWidgetCallback.BindDynamic(this, &UKOUISubsystem::OnCloseWidgetRequest);
        CloseWidgetHandle = GMS->Subscribe(KOGameplayTags::Data_Message_UI_CloseWidget, CloseWidgetCallback);
    }
    else
    {
        UE_LOG(LogKOUI, Warning, TEXT("KOUISubsystem: UGMRouterSubsystem를 찾을 수 없어 Open/Close 구독을 건너뜁니다."));
    }
}

void UKOUISubsystem::Deinitialize()
{
    if (UGMRouterSubsystem* GMS = GetRouter())
    {
        if (OpenWidgetHandle.IsValid())
        {
            GMS->Unsubscribe(OpenWidgetHandle);
        }
        if (CloseWidgetHandle.IsValid())
        {
            GMS->Unsubscribe(CloseWidgetHandle);
        }
    }

    OpenWidgetHandle = FGameplayMessageHandle();
    CloseWidgetHandle = FGameplayMessageHandle();
    OpenWidgetCallback.Clear();
    CloseWidgetCallback.Clear();

    ClearRootLayout();
    ResolvedClassCache.Empty();

    Super::Deinitialize();
}

// ─── Root Layout ──────────────────────────────────────────────────────────────
void UKOUISubsystem::SetRootLayout(FGameplayTag LayoutTag)
{
    if (!LayoutTag.IsValid())
    {
        UE_LOG(LogKOUI, Warning, TEXT("KOUISubsystem::SetRootLayout: 유효하지 않은 LayoutTag입니다."));
        return;
    }

    const UKOUISettings* Settings = UKOUISettings::Get();
    if (!Settings)
    {
        UE_LOG(LogKOUI, Warning, TEXT("KOUISubsystem::SetRootLayout: KOUISettings 접근 실패."));
        return;
    }

    const TSoftClassPtr<UCommonActivatableWidget>* SoftClassPtr = Settings->RootLayoutMap.Find(LayoutTag);
    if (!SoftClassPtr || SoftClassPtr->IsNull())
    {
        UE_LOG(LogKOUI, Warning,
            TEXT("KOUISubsystem::SetRootLayout: RootLayoutMap에 [%s] 매핑이 없거나 클래스가 비어 있습니다."),
            *LayoutTag.ToString());
        return;
    }

    TSubclassOf<UCommonActivatableWidget> LayoutClass = SoftClassPtr->LoadSynchronous();
    if (!LayoutClass)
    {
        UE_LOG(LogKOUI, Warning,
            TEXT("KOUISubsystem::SetRootLayout: 루트 레이아웃 클래스 로드 실패. Tag=%s Path=%s"),
            *LayoutTag.ToString(), *SoftClassPtr->ToString());
        return;
    }

    ULocalPlayer* LP = GetLocalPlayer();
    UGameInstance* GI = LP ? LP->GetGameInstance() : nullptr;
    UWorld* World = GI ? GI->GetWorld() : nullptr;
    APlayerController* PC = LP ? LP->GetPlayerController(World) : nullptr;
    if (!PC)
    {
        UE_LOG(LogKOUI, Warning, TEXT("KOUISubsystem::SetRootLayout: PlayerController를 찾을 수 없습니다."));
        return;
    }

    // 기존 루트 레이아웃 정리 후 재생성.
    ClearRootLayout();

    RootLayoutInstance = CreateWidget<UCommonActivatableWidget>(PC, LayoutClass);
    if (RootLayoutInstance)
    {
        RootLayoutInstance->AddToViewport();

        // CommonUI ActionRouter가 자식 위젯 activation을 input config refresh로 전파하려면
        // 루트가 "receiving input" 상태여야 함. 명시적으로 활성화.
        RootLayoutInstance->ActivateWidget();

        UE_LOG(LogKOUI, Log, TEXT("KOUISubsystem: 루트 레이아웃 생성 [%s]"), *LayoutTag.ToString());
    }
}

void UKOUISubsystem::ClearRootLayout()
{
    if (IsValid(RootLayoutInstance))
    {
        RootLayoutInstance->RemoveFromParent();
    }
    RootLayoutInstance = nullptr;

    // 레이어 컨테이너는 (이제 파괴된) 루트 레이아웃 소유였으므로 함께 비운다.
    Layers.Empty();
    ActiveWidgetsByTag.Empty();
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
UCommonActivatableWidget* UKOUISubsystem::PushToLayer(FGameplayTag LayerTag, TSubclassOf<UCommonActivatableWidget> WidgetClass)
{
    if (!LayerTag.IsValid() || !WidgetClass)
    {
        UE_LOG(LogKOUI, Warning, TEXT("KOUISubsystem::PushToLayer: LayerTag 또는 WidgetClass가 유효하지 않습니다."));
        return nullptr;
    }

    TObjectPtr<UCommonActivatableWidgetContainerBase>* ContainerPtr = Layers.Find(LayerTag);
    if (!ContainerPtr || !IsValid(*ContainerPtr))
    {
        UE_LOG(LogKOUI, Warning,
            TEXT("KOUISubsystem::PushToLayer: 레이어 [%s]가 등록되어 있지 않습니다. SetRootLayout()이 선행돼야 합니다."),
            *LayerTag.ToString());
        return nullptr;
    }

    return (*ContainerPtr)->AddWidget<UCommonActivatableWidget>(WidgetClass);
}

UCommonActivatableWidget* UKOUISubsystem::FindActiveWidget(FGameplayTag WidgetTag) const
{
    if (const TWeakObjectPtr<UCommonActivatableWidget>* WeakPtr = ActiveWidgetsByTag.Find(WidgetTag))
    {
        return WeakPtr->Get();
    }
    return nullptr;
}

UCommonActivatableWidget* UKOUISubsystem::OpenWidget(FGameplayTag WidgetTag)
{
    if (!WidgetTag.IsValid())
    {
        UE_LOG(LogKOUI, Warning, TEXT("KOUISubsystem::OpenWidget: WidgetTag가 유효하지 않습니다."));
        return nullptr;
    }

    // 중복 open 방지 (예: 같은 위젯 재요청).
    if (UCommonActivatableWidget* Existing = FindActiveWidget(WidgetTag))
    {
        return Existing;
    }

    const UKOUISettings* Settings = UKOUISettings::Get();
    if (!Settings)
    {
        UE_LOG(LogKOUI, Warning, TEXT("KOUISubsystem::OpenWidget: KOUISettings 접근 실패."));
        return nullptr;
    }

    const FKOUIWidgetEntry* Entry = Settings->WidgetMap.Find(WidgetTag);
    if (!Entry || !Entry->LayerTag.IsValid())
    {
        UE_LOG(LogKOUI, Warning,
            TEXT("KOUISubsystem::OpenWidget: WidgetMap에 매핑이 없거나 LayerTag가 유효하지 않습니다. Tag=%s"),
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
                TEXT("KOUISubsystem::OpenWidget: WidgetClass 가 비어 있습니다. Tag=%s"),
                *WidgetTag.ToString());
            return nullptr;
        }

        Class = Entry->WidgetClass.LoadSynchronous();
        if (!Class)
        {
            UE_LOG(LogKOUI, Warning,
                TEXT("KOUISubsystem::OpenWidget: WidgetClass 로드 실패. Tag=%s Path=%s"),
                *WidgetTag.ToString(), *Entry->WidgetClass.ToString());
            return nullptr;
        }

        ResolvedClassCache.Add(WidgetTag, Class);
    }

    UCommonActivatableWidget* NewWidget = PushToLayer(Entry->LayerTag, Class);
    if (NewWidget)
    {
        ActiveWidgetsByTag.Add(WidgetTag, NewWidget);

        TWeakObjectPtr<UCommonActivatableWidget> WeakWidget = NewWidget;

        // Stack에서 다른 위젯이 위로 올라오면 이 위젯은 Deactivate(suspend)된다.
        // suspend/pop 구분이 어려우므로 일단 맵에서 빼두고, 다시 Activate되면(resume) 재등록한다.
        NewWidget->OnDeactivated().AddWeakLambda(this, [this, WidgetTag]()
        {
            ActiveWidgetsByTag.Remove(WidgetTag);
        });

        NewWidget->OnActivated().AddWeakLambda(this, [this, WidgetTag, WeakWidget]()
        {
            if (UCommonActivatableWidget* W = WeakWidget.Get())
            {
                ActiveWidgetsByTag.Add(WidgetTag, W);
            }
        });
    }
    return NewWidget;
}

void UKOUISubsystem::CloseWidget(FGameplayTag WidgetTag)
{
    if (UCommonActivatableWidget* Widget = FindActiveWidget(WidgetTag))
    {
        // CommonActivatableWidget을 비활성화하면 소속 Stack/Queue 컨테이너가 자동으로 제거 처리한다.
        Widget->DeactivateWidget();
    }
}

// ─── GMS 콜백 ─────────────────────────────────────────────────────────────────
void UKOUISubsystem::OnOpenWidgetRequest(FGameplayTag Channel, const FInstancedStruct& Payload)
{
    if (const FKOUIWidgetRequest* Request = Payload.GetPtr<FKOUIWidgetRequest>())
    {
        OpenWidget(Request->WidgetTag);
    }
    else
    {
        UE_LOG(LogKOUI, Warning, TEXT("KOUISubsystem: OpenWidget 페이로드 파싱 실패."));
    }
}

void UKOUISubsystem::OnCloseWidgetRequest(FGameplayTag Channel, const FInstancedStruct& Payload)
{
    if (const FKOUIWidgetRequest* Request = Payload.GetPtr<FKOUIWidgetRequest>())
    {
        CloseWidget(Request->WidgetTag);
    }
    else
    {
        UE_LOG(LogKOUI, Warning, TEXT("KOUISubsystem: CloseWidget 페이로드 파싱 실패."));
    }
}
