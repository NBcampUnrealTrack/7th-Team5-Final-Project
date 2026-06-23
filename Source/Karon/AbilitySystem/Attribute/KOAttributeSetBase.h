#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "KOAttributeSetBase.generated.h"

USTRUCT()
struct FKOEffectContext
{
	GENERATED_BODY()

	UAbilitySystemComponent* InstigatorASC        = nullptr;
	AActor*                  Instigator       = nullptr;
	AController*             InstigatorController  = nullptr;
	ACharacter*              InstigatorCharacter   = nullptr;

	UAbilitySystemComponent* TargetASC        = nullptr;
	AActor*                  TargetActor       = nullptr;
	AController*             TargetController  = nullptr;
	ACharacter*              TargetCharacter   = nullptr;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAttributeChanged, float, OldValue, float, NewValue);
UCLASS(Abstract)
class KARON_API UKOAttributeSetBase : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute,float OldValue, float NewValue) override;
	
protected:
	// ─── Utility Functions  ────────────────────────────────────────────────────
	void AdjustCurrentForMaxChange(
		FGameplayAttributeData& Current,
		const FGameplayAttributeData& Max,
		float NewMaxValue,
		const FGameplayAttribute& CurrentAttribute
	);
	
	FKOEffectContext CacheEffectContext(const FGameplayEffectModCallbackData& Data);
	
	AActor* GetAvatarActor() const;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug")
	float MinLogThreshold = 1.f;
};
