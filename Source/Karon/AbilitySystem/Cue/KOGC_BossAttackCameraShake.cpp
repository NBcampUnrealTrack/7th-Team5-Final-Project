#include "AbilitySystem/Cue/KOGC_BossAttackCameraShake.h"

#include "Kismet/GameplayStatics.h"

UKOGC_BossAttackCameraShake::UKOGC_BossAttackCameraShake()
{
}
 
bool UKOGC_BossAttackCameraShake::OnExecute_Implementation(
	AActor* MyTarget,
	const FGameplayCueParameters& Parameters) const
{
	return PlayCameraShake(MyTarget);
}
 
bool UKOGC_BossAttackCameraShake::OnActive_Implementation(
	AActor* MyTarget,
	const FGameplayCueParameters& Parameters) const
{
	return PlayCameraShake(MyTarget);
}
 
bool UKOGC_BossAttackCameraShake::PlayCameraShake(AActor* MyTarget) const
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(MyTarget, 0);
	if (!PC || !ShakeClass)
	{
		return false;
	}
	
	PC->ClientStartCameraShake(ShakeClass, Scale);
	return true;
}
