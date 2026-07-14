// Copyright Karon Team 5. All Rights Reserved.

#include "UI/Title/KOTitleWidget.h"
#include "UI/KOUISubsystem.h"
#include "UI/ConfirmationPopup/KOConfirmationPopup.h"
#include "UI/Loading/KOLoadingUiSubsystem.h"
#include "AbilitySystem/Tag/UI/KOGameplayTags_UI.h"
#include "Subsystem/KOSaveSubsystem.h"

#include "CommonButtonBase.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystem/KOQuestGuideSubsystem.h"

void UKOTitleWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (StartGameButton)
	{
		StartGameButton->OnClicked().AddUObject(this, &UKOTitleWidget::OnStartGameClicked);
	}

	if (QuitGameButton)
	{
		QuitGameButton->OnClicked().AddUObject(this, &UKOTitleWidget::OnQuitGameClicked);
	}

	if (OptionButton)
	{
		OptionButton->OnClicked().AddUObject(this, &UKOTitleWidget::OnOptionClicked);
	}
	
	if (LoadButton)
	{
		LoadButton->OnClicked().AddUObject(this, &UKOTitleWidget::OnLoadClicked);
	}

	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	if (LocalPlayer == nullptr)
	{
		return;
	}
	CachedUISubsystem = LocalPlayer->GetSubsystem<UKOUISubsystem>();
}

void UKOTitleWidget::NativeDestruct()
{
	CachedUISubsystem = nullptr;

	Super::NativeDestruct();
}

void UKOTitleWidget::StartGameConfirmation()
{
	if (UKOSaveSubsystem* SaveSubsystem = UKOSaveSubsystem::Get(this))
	{
		// 이전 진행 세이브 삭제
		SaveSubsystem->DeleteSave();
	}
	
	// 퀘스트 진행 초기화
	if (UKOQuestGuideSubsystem* QuestGuide = UKOQuestGuideSubsystem::Get(this))
	{
		QuestGuide->ResetQuestGuide();
	}
	
	//메인 레벨 변경 시 이름 변경
	FName TargetLevelName = FName("LV_OutfitSelect");
	
	if (auto* LoadingSubsystem = GetGameInstance()->GetSubsystem<UKOLoadingUiSubsystem>())
	{
		LoadingSubsystem->TransitionToLevel(TargetLevelName, LoadingWidget);
	}
}

#define LOCTEXT_NAMESPACE "KOTitleWidget"

void UKOTitleWidget::OnStartGameClicked()
{
	UKOSaveSubsystem* SaveSubsystem = UKOSaveSubsystem::Get(this);

	if (!SaveSubsystem || !SaveSubsystem->DoesSaveExist())
	{
		StartGameConfirmation();
		return;
	}

	if (!CachedUISubsystem)
	{
		return;
	}

	UCommonActivatableWidget* Widget = CachedUISubsystem->OpenWidget(KOGameplayTags::UI_Widget_ConfirmationPopup);

	if (UKOConfirmationPopup* Popup = Cast<UKOConfirmationPopup>(Widget))
	{
		Popup->OnConfirmed.Clear();

		Popup->SetupPopup(
			LOCTEXT("StartNewGameTitle", "새 게임 시작"),
			LOCTEXT(
				"StartNewGameDescription",
				"기존 저장 데이터가 있습니다.\n"
				"새 게임을 시작하면 기존 저장 데이터가 삭제됩니다.\n"
				"계속하시겠습니까?"
			)
		);

		Popup->OnConfirmed.AddUniqueDynamic(this, &UKOTitleWidget::StartGameConfirmation);
	}
}

void UKOTitleWidget::OnQuitGameClicked()
{
	if (!CachedUISubsystem)
	{
		return;
	}

	UCommonActivatableWidget* Widget = CachedUISubsystem->OpenWidget(KOGameplayTags::UI_Widget_ConfirmationPopup);
	if (UKOConfirmationPopup* Popup = Cast<UKOConfirmationPopup>(Widget))
	{
		Popup->OnConfirmed.Clear();
		Popup->SetupPopup(LOCTEXT("QuitGameTitle", "게임 종료"),
		                  LOCTEXT("QuitGameDescription", "게임을 종료하시겠습니까?"));
		Popup->OnConfirmed.AddUniqueDynamic(this, &UKOTitleWidget::GameQuitConfirmation);
	}
}

void UKOTitleWidget::OnLoadClicked() const
{
	UKOSaveSubsystem* SaveSubsystem = UKOSaveSubsystem::Get(this);
	if (!SaveSubsystem)
	{
		return;
	}

	if (!SaveSubsystem->DoesSaveExist())
	{
		UCommonActivatableWidget* Widget = CachedUISubsystem->OpenWidget(KOGameplayTags::UI_Widget_ConfirmationPopup);
		if (UKOConfirmationPopup* Popup = Cast<UKOConfirmationPopup>(Widget))
		{
			Popup->OnConfirmed.Clear();
			Popup->SetupPopup(LOCTEXT("NoSaveFileTitle", "알림"),
				LOCTEXT("NoSaveFileDescription", "불러올 세이브 파일이 없습니다."), false
			);
		}
		return;
	}

	SaveSubsystem->RequestLobbyLoad();

	const FName TargetLevelName = FName("LV_OutfitSelect");

	if (UKOLoadingUiSubsystem* LoadingSubsystem = GetGameInstance()->GetSubsystem<UKOLoadingUiSubsystem>())
	{
		LoadingSubsystem->TransitionToLevel(TargetLevelName, LoadingWidget);
	}
}

#undef LOCTEXT_NAMESPACE

void UKOTitleWidget::OnOptionClicked() const
{
	// KOOptionWidget은 PlayerMenu의 Option 팝업과 동일하게 UI.Widget.Option 태그로 연다.
	UKOUISubsystem::OpenWidget(this, KOGameplayTags::UI_Widget_Option);
}

void UKOTitleWidget::GameQuitConfirmation()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		// 가장 마지막 인자는 강제 종료 여부로 데스크탑/PIE외에도 동작하려면 true가 필요함
		UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, false);
	}
}
