// Copyright Karon Team 5. All Rights Reserved.

#include "KOConfirmationPopup.h"
#include "UI/KOUISubsystem.h"
#include "AbilitySystem/Tag/UI/KOGameplayTags_UI.h"

#include "CommonButtonBase.h"
#include "CommonTextBlock.h"

UKOConfirmationPopup::UKOConfirmationPopup()
{
	InputMode = EKOUIInputMode::Menu;
	bIsBackHandler = true;
}

void UKOConfirmationPopup::SetupPopup(const FText& Title, const FText& Description, bool bShowCancelButton)
{
	if (TitleText)
	{
		TitleText->SetText(Title);
	}

	if (DescriptionText)
	{
		DescriptionText->SetText(Description);
	}

	if (CloseButton)
	{
		CloseButton->SetVisibility(bShowCancelButton ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UKOConfirmationPopup::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (CloseButton)
	{
		CloseButton->OnClicked().AddUObject(this, &UKOConfirmationPopup::OnCloseButtonClicked);
	}

	if (ConfirmButton)
	{
		ConfirmButton->OnClicked().AddUObject(this, &UKOConfirmationPopup::HandleConfirm);
	}
}

void UKOConfirmationPopup::HandleConfirm()
{
	OnConfirmed.Broadcast();
	UKOUISubsystem::CloseWidget(this, KOGameplayTags::UI_Widget_ConfirmationPopup);
}

void UKOConfirmationPopup::OnCloseButtonClicked()
{
	UKOUISubsystem::CloseWidget(this, KOGameplayTags::UI_Widget_ConfirmationPopup);
}
