// Fill out your copyright notice in the Description page of Project Settings.


#include "KOGA_EnemyCounterAttack.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attribute/KOGroggySet.h"
#include "AbilitySystem/Tag/Event/KOGameplayTags_Event.h"
#include "AbilitySystem/Tag/State/KOGameplayTags_State.h"
#include "Character/Enemy/KOBaseEnemy.h"

UKOGA_EnemyCounterAttack::UKOGA_EnemyCounterAttack()
{
	InstancingPolicy  = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = KOGameplayTags::Event_CounterAttack; 
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
	
	ActivationOwnedTags.AddTag(KOGameplayTags::State_Enemy_Attack_CounterAttack);
	
	// 사망 / 패리그로기 중 진입차단 
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Character_Dead);
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Enemy_Parried);
}

void UKOGA_EnemyCounterAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	//실행 전에 히트리액트가 실행되고있다면 즉시 캔슬시킨다.
	if (UAbilitySystemComponent* TargetASC = GetAbilitySystemComponentFromActorInfo())
	{
		FGameplayTagContainer TagsToCancel;
		TagsToCancel.AddTag(KOGameplayTags::State_Enemy_Hit_Normal);

		TargetASC->CancelAbilities(&TagsToCancel);
	}
	
	
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	//Enemy일 경우 CounterAttack 브로드캐스트
	if (AKOBaseEnemy* Enemy= Cast<AKOBaseEnemy>(GetAvatarCharacter()))
	{
		Enemy->OnCounterAttackEvent.ExecuteIfBound(true);	
	}
}

void UKOGA_EnemyCounterAttack::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	//Enemy일 경우 CounterAttack 브로드캐스트
	if (AKOBaseEnemy* Enemy= Cast<AKOBaseEnemy>(GetAvatarCharacter()))
	{
		Enemy->OnCounterAttackEvent.ExecuteIfBound(false);	
	}
	
	//Groggy 초기화
	UAbilitySystemComponent* ASC=GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return;
	}
	// const_cast를 통해 수정 가능하도록 변경
	if (UKOGroggySet* GroggySet=const_cast<UKOGroggySet*>(ASC->GetSet<UKOGroggySet>()))
	{
		GroggySet->SetGroggyHealth(GroggySet->GetMaxGroggyHealth());
	}
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
