#include "KOGameMode.h"

#include "AbilitySystem/Tag/UI/KOGameplayTags_UI.h"
#include "UI/KOUISubsystem.h"

AKOGameMode::AKOGameMode()
{
	bUseSeamlessTravel = true;
}

void AKOGameMode::HandlePlayerDeath(AActor* DeathInstigator)
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC == nullptr) return;

	TWeakObjectPtr<AKOGameMode> WeakThis(this);
	TWeakObjectPtr<APlayerController> WeakPC(PC);

	// 딜레이 후 페이드 아웃 + 레벨 리로드
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, [WeakThis, WeakPC]()
	{
		if (!WeakThis.IsValid() || !WeakPC.IsValid()) return;

		if (APlayerCameraManager* CamMgr = WeakPC->PlayerCameraManager)
		{
			CamMgr->StartCameraFade(0.f, 0.7f, WeakThis->FadeDuration,
			                        FLinearColor::Black, false, true);
		}
	}, RespawnDuration, false);

	if (UKOUISubsystem* UISubsystem = UKOUISubsystem::Get(this))
	{
		UISubsystem->OpenWidget(GetWorld(), KOGameplayTags::UI_Widget_GameOverMenu);
	}
}
