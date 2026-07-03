// Copyright Karon Team 5. All Rights Reserved.

#include "UI/Title/KOTitleWidget.h"
#include "UI/KOUISubsystem.h"
#include "UI/ConfirmationPopup/KOConfirmationPopup.h"
#include "AbilitySystem/Tag/UI/KOGameplayTags_UI.h"

#include "CommonButtonBase.h"
#include "Kismet/GameplayStatics.h"

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

void UKOTitleWidget::OnStartGameClicked() const
{
	//TODO_CSH 메인 레벨 추가시 이름 등록
	FName TargetLevelName = FName("L_MainLevel");
	//아래 경로를 확인해 레벨오픈
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

#define LOCTEXT_NAMESPACE "KOTitleWidget"

void UKOTitleWidget::OnQuitGameClicked()
{
	if (!CachedUISubsystem)
	{
		return;
	}

	UCommonActivatableWidget* Widget = CachedUISubsystem->OpenWidget(KOGameplayTags::UI_Widget_ConfirmationPopup);
	if (UKOConfirmationPopup* Popup = Cast<UKOConfirmationPopup>(Widget))
	{
		Popup->SetupPopup(LOCTEXT("QuitGameTitle", "게임 종료"),
		                  LOCTEXT("QuitGameDescription", "게임을 종료하시겠습니까?"));
		Popup->OnConfirmed.AddUniqueDynamic(this, &UKOTitleWidget::GameQuitConfirmation);
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
