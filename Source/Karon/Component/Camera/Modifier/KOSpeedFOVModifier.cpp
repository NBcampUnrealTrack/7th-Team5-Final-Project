#include "KOSpeedFOVModifier.h"
#include "Character/KOCharacterBase.h"

void UKOSpeedFOVModifier::ProcessFOV_Implementation(float DeltaTime, float& InOutFOV)
{
	if (!OwningCharacter) return;

	const float Speed = OwningCharacter->GetVelocity().Size2D();
	float Alpha = FMath::Clamp(Speed / ReferenceSpeed, 0.f, 1.f);
	if (SpeedToAlphaCurve) Alpha = SpeedToAlphaCurve->GetFloatValue(Alpha);

	const float TargetBoost = Alpha * MaxFOVBoost;
	CurrentBoost = FMath::FInterpTo(CurrentBoost, TargetBoost, DeltaTime, InterpSpeed);

	InOutFOV += CurrentBoost; 
}
