#include "KOCameraModifier.h"
#include "AbilitySystemComponent.h"
#include "Character/KOCharacterBase.h"

bool UKOCameraModifier::IsModifierActive_Implementation() const
{
	if (!bEnabled || !OwningCharacter) return false;
	
	if (!ActivationTag.IsValid()) return true;
	
	if (UAbilitySystemComponent* ASC = OwningCharacter->GetAbilitySystemComponent())
	{
		return ASC->HasMatchingGameplayTag(ActivationTag);
	}

	return false;
}
