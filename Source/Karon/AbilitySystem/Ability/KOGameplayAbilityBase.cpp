#include "KOGameplayAbilityBase.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Tag/Data/KOGameplayTags_Data.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerState.h"
#include "Utility/Log/KOLogManager.h"

void UKOGameplayAbilityBase::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	ACharacter* Character = GetAvatarCharacter();
	if (!Character) return;
	
	KO_LOGS(GAS, Ability, Log, TEXT("(+) %s | %s ← Activated"), *Character->GetName(), *GetClass()->GetName());
}

void UKOGameplayAbilityBase::CancelAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateCancelAbility)
{
	ACharacter* Character = GetAvatarCharacter();
	if (!Character) return;
	
	KO_LOGS(GAS, Ability, Log, TEXT("(!) %s | %s ← Canceled"), *Character->GetName(), *GetClass()->GetName());
	
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

void UKOGameplayAbilityBase::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (CostEffectHandle.IsValid())
	{
		BP_RemoveGameplayEffectFromOwnerWithHandle(CostEffectHandle);
		CostEffectHandle = FActiveGameplayEffectHandle();
	}
	
	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		KO_LOGS(GAS, Ability, Log, TEXT("(-) %s | %s ← Ended%s"),
			*ActorInfo->AvatarActor->GetName(), 
			*GetClass()->GetName(),
			bWasCancelled ? TEXT(" (Cancelled)") : TEXT("")
		);
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

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
	float Amount, float Level)
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
	
	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddSourceObject(GetAvatarCharacter());
	
	FGameplayEffectSpecHandle SpecHandle = 
		SourceASC->MakeOutgoingSpec(EffectClass, Level, Context);
	
	if (!SpecHandle.IsValid()) return FActiveGameplayEffectHandle();
	
	return SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data, TargetASC);
}

FActiveGameplayEffectHandle UKOGameplayAbilityBase::ApplyEffectSetByCallerToTarget(
	AActor* TargetActor,
	TSubclassOf<UGameplayEffect> EffectClass, 
	FGameplayTag DataTag,
	float Amount, float Level)
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

void UKOGameplayAbilityBase::ApplyGameplayCue(FGameplayTag CueTag, FGameplayCueParameters& Parameters)
{
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC || !CueTag.IsValid()) return;
	
	ASC->ExecuteGameplayCue(CueTag, Parameters);
}

void UKOGameplayAbilityBase::ApplyGameplayCues(FGameplayTagContainer CueTag, FGameplayCueParameters& Parameters)
{
	for (auto& CueTag : CueTags)
	{
		if (!CueTag.IsValid()) continue;
		
		ApplyGameplayCue(CueTag, Parameters);
	}
}

UGameplayEffect* UKOGameplayAbilityBase::GetCooldownGameplayEffect() const
{
	if (CooldownGEClass)
		return CooldownGEClass->GetDefaultObject<UGameplayEffect>();
	
	return nullptr;
}

const FGameplayTagContainer* UKOGameplayAbilityBase::GetCooldownTags() const
{ 
	CachedCooldownTags.Reset();
	
	if (CooldownTag.IsValid())
		CachedCooldownTags.AddTag(CooldownTag);
	
	return &CachedCooldownTags;
}

void UKOGameplayAbilityBase::ApplyCooldown(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (!CooldownGEClass || !CooldownTag.IsValid() || CooldownDuration <= 0.f) return;

	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(CooldownGEClass, GetAbilityLevel());
	if (!SpecHandle.IsValid()) return;

	SpecHandle.Data->SetSetByCallerMagnitude(
		KOGameplayTags::Data_CoolTime,
		CooldownDuration
	);
	SpecHandle.Data->DynamicGrantedTags.AddTag(CooldownTag);

	ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
}

UGameplayEffect* UKOGameplayAbilityBase::GetCostGameplayEffect() const
{
	if (CostGEClass)
		return CostGEClass->GetDefaultObject<UGameplayEffect>();
	
	return nullptr;
}

void UKOGameplayAbilityBase::ApplyCost(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (!CostGEClass) return;
	
	FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(CostGEClass, GetAbilityLevel());
	if (!Spec.IsValid()) return;
	
	CostEffectHandle = ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, Spec);
}
