// Copyright Karon Team 5. All Rights Reserved.

#include "UI/RootLayout/KOInGameRootLayout.h"

#include "UI/KOUISubsystem.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"

#include "Widgets/CommonActivatableWidgetContainer.h"

UKOInGameRootLayout::UKOInGameRootLayout()
{
    // 인게임 기본 입력 모드. 인벤토리 등 상위 위젯이 닫혔을 때 이 설정으로 복귀.
    InputMode = EKOUIInputMode::Game;

    // 게임 모드에서 커서를 숨기려면 CapturePermanently 계열이 필수.
    // CommonUI 내부: NoCapture/CaptureDuringMouseDown는 무조건 SetShowMouseCursor(true)로 박음.
    MouseCaptureMode = EMouseCaptureMode::CapturePermanently_IncludingInitialMouseDown;
}

void UKOInGameRootLayout::NativeConstruct()
{
    Super::NativeConstruct();

    ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
    if (LocalPlayer == nullptr)
    {
        return;
    }

    UKOUISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UKOUISubsystem>();
    if (UISubsystem == nullptr)
    {
        return;
    }

    if (GameLayer)
    {
        UISubsystem->RegisterPrimaryLayout(KOGameplayTags::UI_Layer_Game, GameLayer);
    }
    if (GameMenuLayer)
    {
        UISubsystem->RegisterPrimaryLayout(KOGameplayTags::UI_Layer_GameMenu, GameMenuLayer);
    }
    if (MenuLayer)
    {
        UISubsystem->RegisterPrimaryLayout(KOGameplayTags::UI_Layer_Menu, MenuLayer);
    }
    if (ModalLayer)
    {
        UISubsystem->RegisterPrimaryLayout(KOGameplayTags::UI_Layer_Modal, ModalLayer);
    }
}
