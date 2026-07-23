#include "UKOGC_HitCameraShake.h"
#include "GameFramework/Character.h"
#include "Utility/Log/KOLogManager.h"

UUKOGC_HitCameraShake::UUKOGC_HitCameraShake()
{
}

bool UUKOGC_HitCameraShake::OnExecute_Implementation(
	AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (!ShakeClass) return true;

	KO_LOG(GAS, Warning, TEXT("%s"), *GetName());
	
	const ACharacter* Character = Cast<ACharacter>(MyTarget);
	if (!Character) return true;
	
	if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
	{
		if (PC->PlayerCameraManager)
		{
			PC->PlayerCameraManager->StartCameraShake(ShakeClass, ShakeScale);
		}
	}

	return true;
}
