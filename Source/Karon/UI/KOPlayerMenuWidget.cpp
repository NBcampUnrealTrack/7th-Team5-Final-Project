#include "KOPlayerMenuWidget.h"

#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Kismet/GameplayStatics.h"

UKOPlayerMenuWidget::UKOPlayerMenuWidget()
{
	InputMode = EKOUIInputMode::All;
	bIsBackHandler = true;
}

void UKOPlayerMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	if (Button_Inventory)
	{
		Button_Inventory->IsFocusable = false;
		Button_Inventory->OnClicked.AddDynamic(this, &ThisClass::HandleInventoryClicked);
	}

	if (Button_Factory)
	{
		Button_Factory->IsFocusable = false;
		Button_Factory->OnClicked.AddDynamic(this, &ThisClass::HandleFactoryClicked);
	}

	if (Button_Skill)
	{
		Button_Skill->IsFocusable = false;
		Button_Skill->OnClicked.AddDynamic(this, &ThisClass::HandleSkillClicked);
	}

	if (Button_Option)
	{
		Button_Option->IsFocusable = false;
		Button_Option->OnClicked.AddDynamic(this, &ThisClass::HandleOptionClicked);
	}
	
}

void UKOPlayerMenuWidget::NativeOnActivated()
{
	Super::NativeOnActivated();
	
	SetActiveTab(EKOPlayerMenuTab::Inventory);
	
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	
	// 이미 다른 이유로 퍼즈되어 있던 상태면,
	// 이 위젯이 나중에 마음대로 Unpause하지 않도록 기록하지 않는다.
	if (!UGameplayStatics::IsGamePaused(World))
	{
		UGameplayStatics::SetGamePaused(World, true);
		bPausedGameByThisWidget = true;
	}
}

void UKOPlayerMenuWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();
	
	UWorld* World = GetWorld();
	if (World && bPausedGameByThisWidget)
	{
		UGameplayStatics::SetGamePaused(World, false);
		bPausedGameByThisWidget = false;
	}
}

void UKOPlayerMenuWidget::SetActiveTab(EKOPlayerMenuTab Tab)
{
	if (!ContentSwitcher)
	{
		return;
	}

	int32 Index = 0;

	switch (Tab)
	{
	case EKOPlayerMenuTab::Inventory:
		Index = 0;
		break;

	case EKOPlayerMenuTab::FactoryCraft:
		Index = 1;
		break;

	case EKOPlayerMenuTab::SkillTree:
		Index = 2;
		break;

	case EKOPlayerMenuTab::Option:
		Index = 3;
		break;

	default:
		Index = 0;
		break;
	}

	ContentSwitcher->SetActiveWidgetIndex(Index);
}

void UKOPlayerMenuWidget::HandleInventoryClicked()
{
	SetActiveTab(EKOPlayerMenuTab::Inventory);
}

void UKOPlayerMenuWidget::HandleFactoryClicked()
{
	SetActiveTab(EKOPlayerMenuTab::FactoryCraft);
}

void UKOPlayerMenuWidget::HandleSkillClicked()
{
	SetActiveTab(EKOPlayerMenuTab::SkillTree);
}

void UKOPlayerMenuWidget::HandleOptionClicked()
{
	SetActiveTab(EKOPlayerMenuTab::Option);
}
