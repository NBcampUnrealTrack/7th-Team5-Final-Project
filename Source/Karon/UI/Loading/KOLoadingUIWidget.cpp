// Copyright Karon Team 5. All Rights Reserved.

#include "KOLoadingUIWidget.h"
#include "Components/ProgressBar.h"

void UKOLoadingUIWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	CurrentProgress = 0.0f;
	TargetProgress = 0.0f;
	CurrentFillSpeed = 0.0f;
	bFillCompleted = false;

	SetProgressBarPercent(0.0f);
}

void UKOLoadingUIWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	if (bFillCompleted) return;
	if (FMath::IsNearlyEqual(CurrentProgress, TargetProgress)) return;

	CurrentProgress = FMath::FInterpConstantTo(CurrentProgress, TargetProgress, InDeltaTime, CurrentFillSpeed);
	SetProgressBarPercent(CurrentProgress);

	if (FMath::IsNearlyEqual(CurrentProgress, 1.0f))
	{
		bFillCompleted = true;
		OnFillComplete.Broadcast();
	}
}

void UKOLoadingUIWidget::OnLoadingStarted(bool bIsSeamless)
{

}

void UKOLoadingUIWidget::StartFillProgressBar()
{
	
}

void UKOLoadingUIWidget::SetProgressBarPercent(float Percent)
{
	if (ProgressBar)
	{
		ProgressBar->SetPercent(Percent);
	}
}
