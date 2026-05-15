#include "KOAttributeSetBase.h"

#include "GameplayEffectExtension.h"
#include "GameFramework/Character.h"

void UKOAttributeSetBase::AdjustCurrentForMaxChange(
	FGameplayAttributeData& Current, 
	const FGameplayAttributeData& Max,
	float NewMaxValue,
	const FGameplayAttribute& CurrentAttribute)
{
	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (!ASC) return;
	
	const float Ratio = Max.GetCurrentValue() > 0.f ?
	  Current.GetCurrentValue() / Max.GetCurrentValue() : 1.f;
	
	float NewCurrent = NewMaxValue * Ratio;
	ASC->ApplyModToAttributeUnsafe(
		CurrentAttribute, 
		EGameplayModOp::Override,
		NewCurrent
	);
}

FKOEffectContext UKOAttributeSetBase::CacheEffectContext(const FGameplayEffectModCallbackData& Data)
{
	FKOEffectContext Context;
	
	// Source
	UAbilitySystemComponent* SourceASC = 
	   Data.EffectSpec.GetContext()
		   .GetOriginalInstigatorAbilitySystemComponent();
	
	if (SourceASC && SourceASC->AbilityActorInfo.IsValid())
	{
		Context.SourceASC        = SourceASC;
		Context.SourceActor      = SourceASC->GetAvatarActor();
		Context.SourceController = SourceASC->AbilityActorInfo->PlayerController.Get();
		Context.SourceCharacter  = Cast<ACharacter>(Context.SourceActor);
	}
	
	// Target 
	if (Data.Target.AbilityActorInfo.IsValid())
	{
		Context.TargetASC        = const_cast<UAbilitySystemComponent*>(&Data.Target);
		Context.TargetActor      = Data.Target.AbilityActorInfo->AvatarActor.Get();
		Context.TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
		Context.TargetCharacter  = Cast<ACharacter>(Context.TargetActor);
	}
	
	return Context; 
}
