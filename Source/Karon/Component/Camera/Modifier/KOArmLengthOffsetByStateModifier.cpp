#include "KOArmLengthOffsetByStateModifier.h"

#include "AbilitySystemComponent.h"
#include "Character/KOCharacterBase.h"

void UKOArmLengthOffsetByStateModifier::ProcessArmLength_Implementation(float DeltaTime, float& InOutArmLength)
{
	const float Before = InOutArmLength;
	
	float TargetArmLength = DefaultArmLength;

	if (const FKOArmFramingPreset* Preset = FindActivePreset())
	{
		TargetArmLength = Preset->ArmLength;
	}

	InOutArmLength = FMath::FInterpTo(InOutArmLength, TargetArmLength, DeltaTime, InterpSpeed);
	
	UE_LOG(LogTemp, Error, TEXT("ArmLength %.1f -> %.1f (Target: %.1f)"), Before, InOutArmLength, TargetArmLength);
}

void UKOArmLengthOffsetByStateModifier::ProcessBoomOffset_Implementation(float DeltaTime, FVector& InOutOffset)
{
	FVector TargetOffset = DefaultSocketOffset;

	if (const FKOArmFramingPreset* Preset = FindActivePreset())
	{
		TargetOffset = Preset->SocketOffset;
	}

	InOutOffset = FMath::VInterpTo(InOutOffset, TargetOffset, DeltaTime, InterpSpeed);
}

void UKOArmLengthOffsetByStateModifier::ProcessFOV_Implementation(float DeltaTime, float& InOutFOV)
{
	float TargetFOV = DefaultFOV;

	if (const FKOArmFramingPreset* Preset = FindActivePreset())
	{
		TargetFOV = Preset->FOV;
	}

	InOutFOV = FMath::FInterpTo(InOutFOV, TargetFOV, DeltaTime, InterpSpeed);
}

const FKOArmFramingPreset* UKOArmLengthOffsetByStateModifier::FindActivePreset() const
{
	if (!OwningCharacter) return nullptr;

	UAbilitySystemComponent* ASC = OwningCharacter->GetAbilitySystemComponent();
	if (!ASC) return nullptr;

	const FKOArmFramingPreset* BestMatch = nullptr;

	for (const FKOArmFramingPreset& Preset : Presets)
	{
		UE_LOG(LogTemp, Error, TEXT("Checking tag %s -> Match: %d"),
			*Preset.StateTag.ToString(), ASC->HasMatchingGameplayTag(Preset.StateTag));
		
		if (ASC->HasMatchingGameplayTag(Preset.StateTag))
		{
			if (!BestMatch || Preset.Priority > BestMatch->Priority)
			{
				BestMatch = &Preset;
			}
		}
	}
	
	UE_LOG(LogTemp, Error, TEXT("FindActivePreset result: %s"), BestMatch ? TEXT("FOUND") : TEXT("NULL"));
	
	return BestMatch;
}
