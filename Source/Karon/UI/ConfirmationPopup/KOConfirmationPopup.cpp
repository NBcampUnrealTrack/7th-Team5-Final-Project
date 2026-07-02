// Copyright Karon Team 5. All Rights Reserved.

#include "KOConfirmationPopup.h"

#include "CommonButtonBase.h"

void UKOConfirmationPopup::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (CloseButton)
	{
		CloseButton->OnClicked().AddUObject(this, &UKOConfirmationPopup::OnCloseButtonClicked);
	}
}

void UKOConfirmationPopup::OnCloseButtonClicked()
{
	
}
