#include "KOGameMode.h"

#include "Kismet/GameplayStatics.h"

AKOGameMode::AKOGameMode()
{
	bUseSeamlessTravel = true;
}

void AKOGameMode::HandlePlayerDeath(AActor* DeathInstigator)
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;

	// 입력 비활성화
	PC->DisableInput(PC);
	
	TWeakObjectPtr<AKOGameMode> WeakThis(this);
	TWeakObjectPtr<APlayerController> WeakPC(PC);
	
	// 딜레이 후 페이드 아웃 + 레벨 리로드
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, [WeakThis, WeakPC]()
	{
		if (!WeakThis.IsValid() || !WeakPC.IsValid()) return;
		
		if (APlayerCameraManager* CamMgr = WeakPC->PlayerCameraManager)
		{
			CamMgr->StartCameraFade(
				0.f, 1.f, WeakThis->FadeDuration, FLinearColor::Black, false, true);
		}

		// 페이드 완료 후 레벨 리로드
		TWeakObjectPtr<AKOGameMode> WeakThis2 = WeakThis;
		FTimerHandle ReloadTimer;
		WeakThis->GetWorldTimerManager().SetTimer(ReloadTimer, [WeakThis2]()
		   {
			   if (!WeakThis2.IsValid()) return;
			
			   UGameplayStatics::OpenLevel(WeakThis2.Get(), FName(*WeakThis2->GetWorld()->GetName()), true);
		   }, 
		2.f, false);

	}, RespawnDuration, false);
}
