#include "KOGA_EnemyAINotifier.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Character/Enemy/KOBaseEnemy.h"
#include "Perception/AISense_Damage.h"


UKOGA_EnemyAINotifier::UKOGA_EnemyAINotifier()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UKOGA_EnemyAINotifier::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	UAbilityTask_WaitGameplayEvent* DamageTask = 
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Damaged); 
	
	DamageTask->EventReceived.AddDynamic(this, &ThisClass::OnDamageEventReceived);
	DamageTask->ReadyForActivation(); 
}

void UKOGA_EnemyAINotifier::OnDamageEventReceived(FGameplayEventData Payload)
{
	AKOBaseEnemy* Enemy = Cast<AKOBaseEnemy>(GetAvatarCharacter());
	AActor* Attacker = const_cast<AActor*>(Payload.Instigator.Get());
	
	UAISense_Damage::ReportDamageEvent(
			Enemy->GetWorld(),
			Enemy,
			Attacker, 
			Payload.EventMagnitude,
			Attacker->GetActorLocation(),
			Enemy->GetActorLocation()
		);
}
