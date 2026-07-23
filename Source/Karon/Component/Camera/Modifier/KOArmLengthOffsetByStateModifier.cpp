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
		if (ASC->HasMatchingGameplayTag(Preset.StateTag))
		{
			if (!BestMatch || Preset.Priority > BestMatch->Priority)
			{
				BestMatch = &Preset;
			}
		}
	}
	
	return BestMatch;
}
