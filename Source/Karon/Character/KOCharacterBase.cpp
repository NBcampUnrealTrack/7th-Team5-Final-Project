#include "KOCharacterBase.h"
#include "Karon/AbilitySystem/KOAbilitySystemComponent.h"

AKOCharacterBase::AKOCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

UAbilitySystemComponent* AKOCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

