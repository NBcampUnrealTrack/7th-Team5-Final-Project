#include "KOAbilitySystemGlobals.h"
#include "AbilitySystem/Effect/KOGameplayEffectContext.h"

FGameplayEffectContext* UKOAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FKOGameplayEffectContext();
}
