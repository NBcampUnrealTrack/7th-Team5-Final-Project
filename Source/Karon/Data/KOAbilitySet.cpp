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
		
		UE_LOG(LogTemp, Log, TEXT("[AbilitySet] %-30s | Lv.%d | InputTag: %s"),
			   *AbilityEntry.Ability->GetName(),
			   AbilityEntry.AbilityLevel,
			   AbilityEntry.InputTag.IsValid() ? *AbilityEntry.InputTag.ToString() : TEXT("None")); 	
		
		ASC->GiveAbility(AbilitySpec);
	}
}
