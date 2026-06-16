#include "AbilitySystem/Ability/Enemy/Boss/KOGA_BossDeath.h"

#include "AIController.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

UKOGA_BossDeath::UKOGA_BossDeath()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
 
	// 사망 중 다른 GA 차단
	ActivationOwnedTags.AddTag(KOGameplayTags::State_Boss_Dead);
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Boss_Dead);
}
 
void UKOGA_BossDeath::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
 
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
 
	// AI 이동 정지, BT 정지 
	APawn* Pawn = Cast<APawn>(GetAvatarActorFromActorInfo());
	if (Pawn)
	{
		AAIController* AIC = Cast<AAIController>(Pawn->GetController());
		if (AIC)
		{
			AIC->StopMovement();
 
			if (UBehaviorTreeComponent* BTComp =
				Cast<UBehaviorTreeComponent>(AIC->BrainComponent))
			{
				BTComp->StopTree();
			}
		}
	}
	
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
 
void UKOGA_BossDeath::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
 
