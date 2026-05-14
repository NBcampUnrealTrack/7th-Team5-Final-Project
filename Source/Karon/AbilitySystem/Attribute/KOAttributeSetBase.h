#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "KOAttributeSetBase.generated.h"

USTRUCT()
struct FKOEffectContext
{
	GENERATED_BODY()

	UAbilitySystemComponent* SourceASC        = nullptr;
	AActor*                  SourceActor       = nullptr;
	AController*             SourceController  = nullptr;
	ACharacter*              SourceCharacter   = nullptr;

	UAbilitySystemComponent* TargetASC        = nullptr;
	AActor*                  TargetActor       = nullptr;
	AController*             TargetController  = nullptr;
	ACharacter*              TargetCharacter   = nullptr;
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAttributeChanged, float, float);

UCLASS(Abstract)
class KARON_API UKOAttributeSetBase : public UAttributeSet
{
	GENERATED_BODY()
protected:
	// ─── Utility Functions  ────────────────────────────────────────────────────
	void AdjustCurrentForMaxChange(
		FGameplayAttributeData& Current,
		const FGameplayAttributeData& Max,
		float NewMaxValue,
		const FGameplayAttribute& CurrentAttribute
	);
	
	FKOEffectContext CacheEffectContext(const FGameplayEffectModCallbackData& Data);
};
