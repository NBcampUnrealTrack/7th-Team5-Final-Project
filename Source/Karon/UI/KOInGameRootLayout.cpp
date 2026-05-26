// Copyright Karon Team 5. All Rights Reserved.

#include "KOInGameRootLayout.h"

#include "KOUISubsystem.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"

#include "Widgets/CommonActivatableWidgetContainer.h"

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
