
#include "KOGuideWidget.h"
#include "CommonButtonBase.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "UI/KOUISubsystem.h"

UKOGuideWidget::UKOGuideWidget()
{
	InputMode = EKOUIInputMode::Menu;
	bIsBackHandler = true;
}

void UKOGuideWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UKOGuideWidget::NativeOnActivated()
{
	Super::NativeOnActivated();
	
	if (Button_Close)
		Button_Close->OnClicked.AddDynamic(this, &ThisClass::HandleCloseClicked);
	
	GetGuideContent();
}

void UKOGuideWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();
	
	if (Button_Close)
		Button_Close->OnClicked.RemoveDynamic(this, &ThisClass::HandleCloseClicked);
}

void UKOGuideWidget::GetGuideContent()
{
}

void UKOGuideWidget::HandleCloseClicked()
{
	UKOUISubsystem::CloseWidget(this, KOGameplayTags::UI_Widget_Guide);
}
