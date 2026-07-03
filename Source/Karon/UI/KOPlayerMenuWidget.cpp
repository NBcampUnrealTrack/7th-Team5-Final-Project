#include "KOPlayerMenuWidget.h"

#include "AbilitySystem/Tag/UI/KOGameplayTags_UI.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Kismet/GameplayStatics.h"
#include "UI/ConfirmationPopup/KOConfirmationPopup.h"
#include "UI/KOUISubsystem.h"

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

	if (Button_OpenOptionWidget)
	{
		Button_OpenOptionWidget->IsFocusable = false;
		Button_OpenOptionWidget->OnClicked.AddDynamic(this, &ThisClass::HandleOpenOptionWidgetClicked);
	}

	if (Button_BackToTitle)
	{
		Button_BackToTitle->IsFocusable = false;
		Button_BackToTitle->OnClicked.AddDynamic(this, &ThisClass::HandleBackToTitleClicked);
	}
	
	if (Button_Resume)
	{
		Button_Resume->IsFocusable = false;
		Button_Resume->OnClicked.AddDynamic(this, &ThisClass::HandleResumeClicked);
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

bool UKOPlayerMenuWidget::NativeOnHandleBackAction()
{
	DeactivateWidget();
	return true;
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

void UKOPlayerMenuWidget::HandleOpenOptionWidgetClicked()
{
	UKOUISubsystem::OpenWidget(this, KOGameplayTags::UI_Widget_Option);
}

#define LOCTEXT_NAMESPACE "KOPlayerMenuWidget"

void UKOPlayerMenuWidget::HandleBackToTitleClicked()
{
	UCommonActivatableWidget* Widget = UKOUISubsystem::OpenWidget(this, KOGameplayTags::UI_Widget_ConfirmationPopup);
	if (UKOConfirmationPopup* Popup = Cast<UKOConfirmationPopup>(Widget))
	{
		Popup->SetupPopup(
			LOCTEXT("Back To Title", "타이틀로 돌아가기"),
			LOCTEXT("Back To Title Description", "저장하지 않은 진행 상황은 사라집니다. 타이틀로 돌아가시겠습니까?"));
		Popup->OnConfirmed.AddUniqueDynamic(this, &ThisClass::HandleBackToTitleConfirmed);
	}
}

#undef LOCTEXT_NAMESPACE

void UKOPlayerMenuWidget::HandleBackToTitleConfirmed()
{
	FName TargetLevelName = FName("L_MainMenu");
	FString PackagePath = FString::Printf(TEXT("/Game/Karon/Map/%s"), *TargetLevelName.ToString());

	if (FPackageName::DoesPackageExist(PackagePath))
	{
		UGameplayStatics::OpenLevel(GetWorld(), TargetLevelName);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("오류: '%s' 레벨을 찾을 수 없습니다! 경로나 이름을 확인하세요."), *PackagePath);
	}
}

void UKOPlayerMenuWidget::HandleResumeClicked()
{
	UKOUISubsystem::CloseWidget(this, KOGameplayTags::UI_Widget_PlayerMenu);
}
