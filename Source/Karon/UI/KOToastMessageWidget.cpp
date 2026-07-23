#include "KOToastMessageWidget.h"

#include "Components/TextBlock.h"
#include "HAL/PlatformTime.h"

void UKOToastMessageWidget::NativeConstruct()
{
	Super::NativeConstruct();

	bMessageVisible = false;
	HideTimeSeconds = 0.0;

	if (MessageText)
	{
		MessageText->SetText(FText::GetEmpty());
	}

	SetVisibility(ESlateVisibility::Collapsed);
}

void UKOToastMessageWidget::NativeDestruct()
{
	bMessageVisible = false;
	HideTimeSeconds = 0.0;

	Super::NativeDestruct();
}

void UKOToastMessageWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bMessageVisible)
	{
		return;
	}

	if (FPlatformTime::Seconds() >= HideTimeSeconds)
	{
		HideMessage();
	}
}

void UKOToastMessageWidget::ShowMessage(const FText& InMessage, float InDuration)
{
	if (MessageText)
	{
		MessageText->SetText(InMessage);
		MessageText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	const float Duration = InDuration > 0.0f ? InDuration : DefaultDuration;

	bMessageVisible = true;
	HideTimeSeconds = FPlatformTime::Seconds() + Duration;

	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UKOToastMessageWidget::HideMessage()
{
	bMessageVisible = false;
	HideTimeSeconds = 0.0;

	if (MessageText)
	{
		MessageText->SetText(FText::GetEmpty());
	}

	SetVisibility(ESlateVisibility::Collapsed);

	OnToastFinished.Broadcast();
}