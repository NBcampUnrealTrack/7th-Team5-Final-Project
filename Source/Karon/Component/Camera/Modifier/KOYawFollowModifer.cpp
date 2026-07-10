#include "KOYawFollowModifer.h"

#include "Character/KOCharacterBase.h"
#include "Game/KOPlayerController.h"

bool UKOYawFollowModifer::IsModifierActive_Implementation() const
{
	if (!Super::IsModifierActive_Implementation()) return false;

	AController* Controller = OwningCharacter->GetController();
	if (!Controller) return true; 
	
	AKOPlayerController* PC = Cast<AKOPlayerController>(Controller);
	if (!PC) return true;

	return PC->GetTimeSinceLastLookInput() > ReactivateDelay;
}

void UKOYawFollowModifer::ProcessControlRotation_Implementation(float DeltaTime, FRotator& InOutRotation)
{
	if (!OwningCharacter) return;

	FVector Velocity = OwningCharacter->GetVelocity();
	if (Velocity.SizeSquared2D() < 100.f) return;

	FRotator TargetYaw = Velocity.ToOrientationRotator();
	InOutRotation.Yaw = FMath::RInterpTo(InOutRotation, TargetYaw, DeltaTime, FollowInterpSpeed).Yaw;
}
