// Copyright Karon Team 5. All Rights Reserved.

#include "KOGameOverWidget.h"
#include "UI/Loading/KOLoadingUiSubsystem.h"

#include "CommonButtonBase.h"
#include "AbilitySystem/Tag/UI/KOGameplayTags_UI.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystem/KOSaveSubsystem.h"
#include "UI/KOUISubsystem.h"
#include "GameFramework/PlayerController.h"
#include "Character/Enemy/KOBaseEnemyAIController.h"
#include "EngineUtils.h"
#include "Character/Hero/KOHeroCharacter.h"

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
	if (auto* LoadingSubsystem = GetGameInstance()->GetSubsystem<UKOLoadingUiSubsystem>())
	{
		LoadingSubsystem->ShowLoadingScreen(DefaultLoadingWidget);
	}
	
	UKOSaveSubsystem* SaveSubsystem = UKOSaveSubsystem::Get(this);
	if (!SaveSubsystem)
	{
		if (auto* LoadingSubsystem = GetGameInstance()->GetSubsystem<UKOLoadingUiSubsystem>())
		{
			LoadingSubsystem->HideLoadingScreen();
		}

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
			if (auto* LoadingSubsystem = GetGameInstance()->GetSubsystem<UKOLoadingUiSubsystem>())
			{
				LoadingSubsystem->HideLoadingScreen();
			}

			return;
		}

		AKOHeroCharacter* HeroCharacter = Cast<AKOHeroCharacter>(PlayerController->GetPawn());

		if (!HeroCharacter)
		{
			if (auto* LoadingSubsystem = GetGameInstance()->GetSubsystem<UKOLoadingUiSubsystem>())
			{
				LoadingSubsystem->HideLoadingScreen();
			}

			return;
		}

		// 모든 적의 기존 플레이어 감지 상태 초기화
		for (TActorIterator<AKOBaseEnemyAIController> It(GetWorld()); It; ++It)
		{
			if (AKOBaseEnemyAIController* EnemyController = *It)
			{
				EnemyController->ResetPlayerDetection();
			}
		}

		// 기본값은 현재 위치
		FTransform RespawnTransform = HeroCharacter->GetActorTransform();

		// PlayerStart 위치 사용
		if (AGameModeBase* GameMode = GetWorld()->GetAuthGameMode<AGameModeBase>())
		{
			if (AActor* StartSpot = GameMode->FindPlayerStart(PlayerController))
			{
				RespawnTransform = StartSpot->GetActorTransform();
			}
		}

		// 상태, 위치 복구
		HeroCharacter->RespawnWithoutSave(RespawnTransform);

		// 카메라 페이드 초기화
		if (APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager)
		{
			CameraManager->StopCameraFade();
			CameraManager->SetManualCameraFade(0.0f, FLinearColor::Black, false);
		}

		UKOUISubsystem::CloseWidget(this, KOGameplayTags::UI_Widget_GameOverMenu);

		if (auto* LoadingSubsystem = GetGameInstance()->GetSubsystem<UKOLoadingUiSubsystem>())
		{
			LoadingSubsystem->HideLoadingScreen();
		}

		return;
	}

	// 저장된 게임이 있으면 마지막 저장 시점 로드
	if (!SaveSubsystem->LoadCurrentGame())
	{
		if (auto* LoadingSubsystem = GetGameInstance()->GetSubsystem<UKOLoadingUiSubsystem>())
		{
			LoadingSubsystem->HideLoadingScreen();
		}
		
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
		
	if (auto* LoadingSubsystem = GetGameInstance()->GetSubsystem<UKOLoadingUiSubsystem>())
	{
		LoadingSubsystem->HideLoadingScreen();
	}
}
