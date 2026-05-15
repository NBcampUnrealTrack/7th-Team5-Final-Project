#include "KOAbilitySet.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"

UKOAbilitySet::UKOAbilitySet(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

void UKOAbilitySet::GiveAbilities(UAbilitySystemComponent* ASC) const
{
	if (!ASC) return;
	
	for (const FKOAbilityEntry& AbilityEntry : GrantedAbilities)
	{
		if (!IsValid(AbilityEntry.Ability)) continue;
		
		FGameplayAbilitySpec AbilitySpec(AbilityEntry.Ability, AbilityEntry.AbilityLevel);
		if (AbilityEntry.InputTag.IsValid())
		{
			AbilitySpec.DynamicAbilityTags.AddTag(AbilityEntry.InputTag);
		}
		
		ASC->GiveAbility(AbilitySpec);
	}
}
