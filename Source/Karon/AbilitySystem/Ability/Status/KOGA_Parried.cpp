// Fill out your copyright notice in the Description page of Project Settings.


#include "KOGA_Parried.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/Tag/Event/KOGameplayTags_Event.h"
#include "AbilitySystem/Tag/State/KOGameplayTags_State.h"
#include "Character/Enemy/KOBaseEnemy.h"

UKOGA_Parried::UKOGA_Parried()
{
	InstancingPolicy  = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = KOGameplayTags::Event_Parried; 
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
	
	// 사망 / 무적 / 슈퍼아머 / 에너미히트 중 진입차단 
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Character_Dead);
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Character_Invincible);
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Character_SuperArmor);
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Enemy_Hit_Normal);
}

void UKOGA_Parried::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle, 
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
	)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	UAnimMontage* Montage = ParriedMontage;
	if (!Montage)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	
	//Enemy일 경우 Hit 브로드캐스트
	AKOBaseEnemy* Enemy = Cast<AKOBaseEnemy>(GetAvatarCharacter());
	if (!Enemy)
	{
		return;
	}
	
	Enemy->OnHitEvent.ExecuteIfBound(true);	
	
	
	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			Montage,
			1.0f,
			NAME_None,
			false
		);
	
	if (MontageTask)
	{
		MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
		MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);
		MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCancelled);
		MontageTask->ReadyForActivation();
		GetWorld()->GetTimerManager().SetTimer(
			RecoverTimerHandle,
			this,
			&ThisClass::OnRecoverTimeEnd,
			RecoverInterval,
			false
			);
	}
}

void UKOGA_Parried::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	//Enemy일 경우 Hit false
	if (AKOBaseEnemy* Enemy= Cast<AKOBaseEnemy>(GetAvatarCharacter()))
	{
		Enemy->OnHitEvent.ExecuteIfBound(false);
	}
	GetWorld()->GetTimerManager().ClearTimer(RecoverTimerHandle);
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UKOGA_Parried::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UKOGA_Parried::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UKOGA_Parried::OnRecoverTimeEnd()
{
	// 에너미 및 AnimInstance 가져오기
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	AKOBaseEnemy* Enemy = Cast<AKOBaseEnemy>(AvatarActor);
    
	if (Enemy)
	{
		if (UAnimInstance* AnimInstance = Enemy->GetMesh()->GetAnimInstance())
		{
			if (AnimInstance->Montage_IsActive(ParriedMontage))
			{
				// 그로기 루프 탈출
				AnimInstance->Montage_JumpToSection(ParriedStandUpSectionName, ParriedMontage);
			}
		}
	}
}
