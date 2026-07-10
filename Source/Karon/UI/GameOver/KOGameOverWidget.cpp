// Copyright Karon Team 5. All Rights Reserved.

#include "KOGameOverWidget.h"
#include "UI/Loading/KOLoadingUiSubsystem.h"

#include "CommonButtonBase.h"
#include "AbilitySystem/Tag/UI/KOGameplayTags_UI.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystem/KOSaveSubsystem.h"
#include "UI/KOUISubsystem.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Character/Enemy/KOBaseEnemyAIController.h"
#include "EngineUtils.h"

void UKOGameOverWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (BackToTitleButton)
	{
		BackToTitleButton->OnClicked().AddUObject(this, &ThisClass::OnBackToTitleClicked);
	}
	
	if (BackToLastSaveButton)
	{
		BackToLastSaveButton->OnClicked().AddUObject(this, &ThisClass::OnBackToLastSaveClicked);
	}
}

void UKOGameOverWidget::OnBackToTitleClicked()
{
	FName TargetLevelName = FName("L_MainMenu");
	if (auto* LoadingSubsystem = GetGameInstance()->GetSubsystem<UKOLoadingUiSubsystem>())
	{
		LoadingSubsystem->TransitionToLevel(TargetLevelName, DefaultLoadingWidget);
	}
}

void UKOGameOverWidget::OnBackToLastSaveClicked()
{
	UKOSaveSubsystem* SaveSubsystem = UKOSaveSubsystem::Get(this);
	if (!SaveSubsystem)
	{
		return;
	}

	// 전투 모드 강제 해제
	SaveSubsystem->ForceEndCombat();

	// 저장된 게임이 없으면 처음으로 리스폰
	if (!SaveSubsystem->DoesSaveExist())
	{
		APlayerController* PlayerController = GetOwningPlayer();
		if (!PlayerController)
		{
			return;
		}
		
		for (TActorIterator<AKOBaseEnemyAIController> It(GetWorld()); It; ++It)
		{
			if (AKOBaseEnemyAIController* EnemyController = *It)
			{
				EnemyController->ResetPlayerDetection();
			}
		}

		if (APawn* OldPawn = PlayerController->GetPawn())
		{
			PlayerController->UnPossess();
			OldPawn->Destroy();
		}

		if (AGameModeBase* GameMode = GetWorld()->GetAuthGameMode<AGameModeBase>())
		{
			GameMode->RestartPlayer(PlayerController);
		}
		
		if (APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager)
		{
			CameraManager->StopCameraFade();
			CameraManager->SetManualCameraFade(0.0f, FLinearColor::Black, false);
		}

		UKOUISubsystem::CloseWidget(this, KOGameplayTags::UI_Widget_GameOverMenu);

		return;
	}

	// 저장된 게임이 있으면 마지막 저장 시점 로드
	if (!SaveSubsystem->LoadCurrentGame())
	{
		return;
	}
	
	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		if (APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager)
		{
			CameraManager->StopCameraFade();
			CameraManager->SetManualCameraFade(0.0f, FLinearColor::Black, false);
		}
	}
	
	UKOUISubsystem::CloseWidget(this, KOGameplayTags::UI_Widget_GameOverMenu);
}
