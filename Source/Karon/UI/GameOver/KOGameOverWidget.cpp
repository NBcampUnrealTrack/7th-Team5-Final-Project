// Copyright Karon Team 5. All Rights Reserved.

#include "KOGameOverWidget.h"
#include "UI/Loading/KOLoadingUiSubsystem.h"

#include "CommonButtonBase.h"

void UKOGameOverWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (BackToTitleButton)
	{
		BackToTitleButton->OnClicked().AddUObject(this, &ThisClass::OnBackToTitleClicked);
	}
	
	if (BackToLastSaveButton)
	{
		BackToLastSaveButton->OnClicked().AddUObject(this, &ThisClass::OnBackToLastSaveClicked);
	}
}

void UKOGameOverWidget::OnBackToTitleClicked()
{
	FName TargetLevelName = FName("L_MainMenu");
	if (auto* LoadingSubsystem = GetGameInstance()->GetSubsystem<UKOLoadingUiSubsystem>())
	{
		LoadingSubsystem->TransitionToLevel(TargetLevelName, DefaultLoadingWidget);
	}
}

void UKOGameOverWidget::OnBackToLastSaveClicked()
{
}
