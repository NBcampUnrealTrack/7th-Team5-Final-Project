#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "KOAbilitySystemGlobals.generated.h"


UCLASS()
class KARON_API UKOAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()
	
	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
};
