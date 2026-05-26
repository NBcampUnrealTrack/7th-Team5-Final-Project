#include "KOGameplayAbilityBase.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerState.h"

ACharacter* UKOGameplayAbilityBase::GetAvatarCharacter() const
{
	return Cast<ACharacter>(GetAvatarActorFromActorInfo());
}

AController* UKOGameplayAbilityBase::GetAvatarController() const
{
	if (const ACharacter* Character = GetAvatarCharacter())
		return Character->GetController();
	
	return nullptr;
}

APlayerState* UKOGameplayAbilityBase::GetPlayerState() const
{
	if (const ACharacter* Character = GetAvatarCharacter())
		return Character->GetPlayerState();
	
	return nullptr;
}

UAbilitySystemComponent* UKOGameplayAbilityBase::GetASC() const
{
	return GetAbilitySystemComponentFromActorInfo();
}

bool UKOGameplayAbilityBase::HasMatchingTag(FGameplayTag Tag) const
{
	const UAbilitySystemComponent* ASC = GetASC();
	
	return ASC && ASC->HasMatchingGameplayTag(Tag);
}

bool UKOGameplayAbilityBase::HasAllTags(const FGameplayTagContainer& Tags) const
{
	const UAbilitySystemComponent* ASC = GetASC();
	
	return ASC && ASC->HasAllMatchingGameplayTags(Tags);
}

bool UKOGameplayAbilityBase::HasAnyTags(const FGameplayTagContainer& Tags) const
{
	const UAbilitySystemComponent* ASC = GetASC();
	
	return ASC && ASC->HasAnyMatchingGameplayTags(Tags);
}

FActiveGameplayEffectHandle UKOGameplayAbilityBase::ApplyEffectToSelf(
	TSubclassOf<UGameplayEffect> EffectClass,
	float Level)
{
	if (!EffectClass) return FActiveGameplayEffectHandle();
	
	FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(EffectClass, Level);
	if (!Spec.IsValid()) return FActiveGameplayEffectHandle();
	
	return ApplyGameplayEffectSpecToOwner(
	  CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, Spec);
}

FActiveGameplayEffectHandle UKOGameplayAbilityBase::ApplyEffectSetByCallerToSelf(
	TSubclassOf<UGameplayEffect> EffectClass,
	FGameplayTag DataTag,
	float Amount,
	float Level)
{
	if (!EffectClass) return FActiveGameplayEffectHandle();
	
	FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(EffectClass, Level);
	if (!Spec.IsValid()) return FActiveGameplayEffectHandle();
	
	Spec.Data->SetSetByCallerMagnitude(DataTag, Amount); 

	return ApplyGameplayEffectSpecToOwner(
		CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, Spec);
}


FActiveGameplayEffectHandle UKOGameplayAbilityBase::ApplyEffectToTarget(
	AActor* TargetActor,
	TSubclassOf<UGameplayEffect> EffectClass,
	float Level)
{
	UAbilitySystemComponent* SourceASC = GetASC();
	if (!SourceASC || !EffectClass || !TargetActor) return FActiveGameplayEffectHandle();

	UAbilitySystemComponent* TargetASC =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC) return FActiveGameplayEffectHandle();
	
	FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(EffectClass, Level);
	if (!Spec.IsValid()) return FActiveGameplayEffectHandle();
	
	return SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data, TargetASC);
}

FActiveGameplayEffectHandle UKOGameplayAbilityBase::ApplyEffectSetByCallerToTarget(
	AActor* TargetActor,
	TSubclassOf<UGameplayEffect> EffectClass, 
	FGameplayTag DataTag,
	float Amount,
	float Level)
{
	UAbilitySystemComponent* SourceASC = GetASC();
	if (!SourceASC || !EffectClass || !TargetActor) return FActiveGameplayEffectHandle();

	UAbilitySystemComponent* TargetASC =
	UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC) return FActiveGameplayEffectHandle();
	
	FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(EffectClass, Level);
	if (!Spec.IsValid()) return FActiveGameplayEffectHandle();
	
	Spec.Data->SetSetByCallerMagnitude(DataTag, Amount);

	return SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data, TargetASC);
}
