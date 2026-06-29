#include "KOAttributeSetBase.h"

#include "GameplayEffectExtension.h"
#include "GameFramework/Character.h"
#include "Utility/Log/KOLogManager.h"

void UKOAttributeSetBase::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
	
	if (FMath::Abs(NewValue - OldValue) < MinLogThreshold) return; 
	
	KO_LOGS(GAS, Attribute, Log, TEXT("[%s] %s | %s : %.1f → %.1f"),
		*GetOwningActor()->GetName(),
		*GetClass()->GetName(),
		*Attribute.GetName(),
		OldValue,
		NewValue
	); 
}

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
		Context.InstigatorASC        = SourceASC;
		Context.Instigator      = SourceASC->GetAvatarActor();
		Context.InstigatorController = SourceASC->AbilityActorInfo->PlayerController.Get();
		Context.InstigatorCharacter  = Cast<ACharacter>(Context.Instigator);
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

AActor* UKOAttributeSetBase::GetAvatarActor() const
{
	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	return ASC ? ASC->GetAvatarActor() : nullptr;
}
