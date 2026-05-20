#include "KOGA_Attack_Light.h"

#include "AbilitySystem/Tag/KOGameplayTags.h"

UKOGA_Attack_Light::UKOGA_Attack_Light()
{
	SetAssetTags(FGameplayTagContainer(KOGameplayTags::Input_Ability_Attack_Light));
}

void UKOGA_Attack_Light::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UKOGA_Attack_Light::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
