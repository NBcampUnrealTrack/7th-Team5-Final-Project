#include "KOTitleWidget.h"

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
	// 1. 입력된 레벨 이름을 기반으로 엔진이 인식하는 표준 패키지 경로로 변환합니다.
	// 예: "Lvl_ProductionHub" -> "/Game/Maps/Lvl_ProductionHub" (폴더 구조에 따름)
	FString PackagePath = FPackageName::LongPackageNameToFilename(TargetLevelName.ToString());
	// 만약 폴더 구조가 명확하다면 직접 경로를 조립해도 됩니다.
	// FString PackagePath = FString::Printf(TEXT("/Game/Maps/%s"), *TargetLevelName.ToString());
	// 2. 해당 경로에 실제 맵 패키지(.umap)가 존재하는지 검사
	if (FPackageName::DoesPackageExist(PackagePath))
	{
		UGameplayStatics::OpenLevel(this, TargetLevelName);
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
