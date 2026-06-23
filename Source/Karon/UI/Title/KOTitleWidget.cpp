#include "UI/Title/KOTitleWidget.h"

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

void UKOTitleWidget::OnQuitGameClicked() const
{
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		// 가장 마지막 인자는 강제 종료 여부로 데스크탑/PIE외에도 동작하려면 true가 필요함
		UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, false);
	}
}
