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
	return Cast<APlayerState>(GetOwningActorFromActorInfo());
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
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC || !EffectClass) return FActiveGameplayEffectHandle();
    
	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	Context.AddInstigator(GetOwningActorFromActorInfo(), GetAvatarActorFromActorInfo());
    
	FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(EffectClass, Level, Context);
	if (!Spec.IsValid()) return FActiveGameplayEffectHandle();
    
	return ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data);
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

	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddInstigator(GetOwningActorFromActorInfo(), GetAvatarActorFromActorInfo());

	FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(EffectClass, Level, Context);
	if (!Spec.IsValid()) return FActiveGameplayEffectHandle();

	return SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data, TargetASC);
}
