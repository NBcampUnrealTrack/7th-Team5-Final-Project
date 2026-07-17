#include "KOPlayerMenuWidget.h"

#include "KOToastMessageWidget.h"
#include "AbilitySystem/Tag/UI/KOGameplayTags_UI.h"
#include "UI/ConfirmationPopup/KOConfirmationPopup.h"
#include "UI/KOUISubsystem.h"
#include "UI/Loading/KOLoadingUiSubsystem.h"

#include "Components/Button.h"
#include "Groups/CommonButtonGroupBase.h"
#include "CommonButtonBase.h"
#include "Components/WidgetSwitcher.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystem/KOSaveSubsystem.h"

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
		Button_Inventory->SetIsFocusable(false);
		Button_Inventory->OnClicked().AddUObject(this, &ThisClass::HandleInventoryClicked);
	}

	if (Button_Factory)
	{
		Button_Factory->SetIsFocusable(false);
		Button_Factory->OnClicked().AddUObject(this, &ThisClass::HandleFactoryClicked);
	}

	if (Button_Skill)
	{
		Button_Skill->SetIsFocusable(false);
		Button_Skill->OnClicked().AddUObject(this, &ThisClass::HandleSkillClicked);
	}

	if (Button_Option)
	{
		Button_Option->SetIsFocusable(false);
		Button_Option->OnClicked().AddUObject(this, &ThisClass::HandleOptionClicked);
	}
	
	if (ToastMessageWidget)
	{
		ToastMessageWidget->OnToastFinished.AddUniqueDynamic(this, &ThisClass::HandleToastFinished);
	}

	if (Button_OpenOptionWidget)
	{
		Button_OpenOptionWidget->IsFocusable = false;
		Button_OpenOptionWidget->OnClicked.AddDynamic(this, &ThisClass::HandleOpenOptionWidgetClicked);
	}
	
	if (Button_Save)
	{
		Button_Save->IsFocusable = false;
		Button_Save->OnClicked.AddDynamic(this, &ThisClass::HandleSaveClicked);
	}

	if (Button_Load)
	{
		Button_Load->IsFocusable = false;
		Button_Load->OnClicked.AddDynamic(this, &ThisClass::HandleLoadClicked);
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
	
	if (Button_QuitGame)
	{
		Button_QuitGame->IsFocusable = false;
		Button_QuitGame->OnClicked.AddDynamic(this, &ThisClass::HandleQuitGameClicked);
	}
	
	// 버큰 그룹 등록
	TabButtonGroup = NewObject<UCommonButtonGroupBase>(this);

	if (TabButtonGroup)
	{
		TabButtonGroup->SetSelectionRequired(true);

		TabButtonGroup->AddWidget(Button_Inventory);
		TabButtonGroup->AddWidget(Button_Factory);
		TabButtonGroup->AddWidget(Button_Skill);
		TabButtonGroup->AddWidget(Button_Option);
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

	if (TabButtonGroup)
	{
		TabButtonGroup->SelectButtonAtIndex(Index, false);
	}
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

void UKOPlayerMenuWidget::HandleSaveClicked()
{
	UKOSaveSubsystem* SaveSubsystem = UKOSaveSubsystem::Get(this);
	if (!SaveSubsystem)
	{
		ShowLocalMessage(FText::FromString(TEXT("저장 실패")));
		return;
	}
	
	if (!SaveSubsystem->CanSaveOrLoad())
	{
		ShowLocalMessage(FText::FromString(TEXT("전투 중에는 저장할 수 없습니다.")));
		return;
	}

	const bool bSaved =	SaveSubsystem && SaveSubsystem->SaveCurrentGame();

	ShowLocalMessage(bSaved ? FText::FromString(TEXT("저장 완료")) : FText::FromString(TEXT("저장 실패")));
}

void UKOPlayerMenuWidget::HandleLoadClicked()
{
	UKOSaveSubsystem* SaveSubsystem = UKOSaveSubsystem::Get(this);
	if (!SaveSubsystem)
	{
		ShowLocalMessage(FText::FromString(TEXT("로드 실패")));
		return;
	}

	if (!SaveSubsystem->CanSaveOrLoad())
	{
		ShowLocalMessage(FText::FromString(TEXT("전투 중에는 로드할 수 없습니다.")));
		return;
	}
	
	// 저장 파일이 없으면 로딩 화면을 띄우지 않고 종료
	if (!SaveSubsystem->DoesSaveExist())
	{
		ShowLocalMessage(FText::FromString(TEXT("저장된 게임이 없습니다.")));
		return;
	}
	
	UKOLoadingUiSubsystem* LoadingSubsystem =
		GetGameInstance()
		? GetGameInstance()->GetSubsystem<UKOLoadingUiSubsystem>()
		: nullptr;

	if (LoadingSubsystem)
	{
		LoadingSubsystem->ShowLoadingScreen(DefaultLoadingWidget);
	}

	const bool bLoaded = SaveSubsystem && SaveSubsystem->LoadCurrentGame();
	
	if (!bLoaded)
	{
		if (LoadingSubsystem)
		{
			LoadingSubsystem->HideLoadingScreen();
		}
		
		ShowLocalMessage(FText::FromString(TEXT("로드 실패")));
		return;
	}

	UKOUISubsystem::CloseWidget(this, KOGameplayTags::UI_Widget_PlayerMenu);
	
	if (LoadingSubsystem)
	{
		LoadingSubsystem->HideLoadingScreen();
	}
}

#define LOCTEXT_NAMESPACE "KOPlayerMenuWidget"

void UKOPlayerMenuWidget::HandleBackToTitleClicked()
{
	UCommonActivatableWidget* Widget = UKOUISubsystem::OpenWidget(this, KOGameplayTags::UI_Widget_ConfirmationPopup);
	if (UKOConfirmationPopup* Popup = Cast<UKOConfirmationPopup>(Widget))
	{
		Popup->OnConfirmed.Clear();
		Popup->SetupPopup(
			LOCTEXT("Back To Title", "타이틀로 돌아가기"),
			LOCTEXT("Back To Title Description", "저장하지 않은 진행 상황은 사라집니다. 타이틀로 돌아가시겠습니까?"));
		Popup->OnConfirmed.AddUniqueDynamic(this, &ThisClass::HandleBackToTitleConfirmed);
	}
}

void UKOPlayerMenuWidget::HandleQuitGameClicked()
{
	UCommonActivatableWidget* Widget = UKOUISubsystem::OpenWidget(this, KOGameplayTags::UI_Widget_ConfirmationPopup);
	if (UKOConfirmationPopup* Popup = Cast<UKOConfirmationPopup>(Widget))
	{
		Popup->OnConfirmed.Clear();
		Popup->SetupPopup(
		LOCTEXT("Quit Game", "게임 종료"),	
		LOCTEXT("Quit Game Description", "저장하지 않은 진행 상황은 사라집니다. \n정말 게임을 종료하시겠습니까?"));
		Popup->OnConfirmed.AddUniqueDynamic(this, &ThisClass::HandleQuitGameConfirmed);
	}
}

#undef LOCTEXT_NAMESPACE

void UKOPlayerMenuWidget::HandleBackToTitleConfirmed()
{
	FName TargetLevelName = FName("L_MainMenu");
	if (auto* LoadingSubsystem = GetGameInstance()->GetSubsystem<UKOLoadingUiSubsystem>())
	{
		LoadingSubsystem->TransitionToLevel(TargetLevelName, DefaultLoadingWidget);
	}
}

void UKOPlayerMenuWidget::HandleResumeClicked()
{
	UKOUISubsystem::CloseWidget(this, KOGameplayTags::UI_Widget_PlayerMenu);
}

void UKOPlayerMenuWidget::HandleQuitGameConfirmed()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		// 가장 마지막 인자는 강제 종료 여부로 데스크탑/PIE외에도 동작하려면 true가 필요함
		UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, false);
	}
}

void UKOPlayerMenuWidget::HandleToastFinished()
{
	if (!bCloseMenuAfterToast)
	{
		return;
	}

	bCloseMenuAfterToast = false;
	UKOUISubsystem::CloseWidget(this, KOGameplayTags::UI_Widget_PlayerMenu);
}

void UKOPlayerMenuWidget::ShowLocalMessage(const FText& Message, bool bCloseAfterMessage)
{
	bCloseMenuAfterToast = bCloseAfterMessage;

	if (ToastMessageWidget)
	{
		ToastMessageWidget->ShowMessage(Message, 2.0f);
	}
	
	if (bCloseAfterMessage)
	{
		bCloseMenuAfterToast = false;
		UKOUISubsystem::CloseWidget(this, KOGameplayTags::UI_Widget_PlayerMenu);
	}
}
