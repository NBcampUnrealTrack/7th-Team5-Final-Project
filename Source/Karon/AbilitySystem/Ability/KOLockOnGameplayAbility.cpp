// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Ability/KOLockOnGameplayAbility.h"
#include "Component/KOLockOnComponent.h"
#include "GameFramework/Character.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"

UKOLockOnGameplayAbility::UKOLockOnGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	// Input.Ability.LockOn 태그로 수정
	SetAssetTags(FGameplayTagContainer(KOGameplayTags::Input_Ability_LockOn));
}

void UKOLockOnGameplayAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	UKOLockOnComponent* LockOnComp = 
		Character->FindComponentByClass<UKOLockOnComponent>();
	if (!LockOnComp)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	// 토글
	if (LockOnComp->IsLockedOn())
		LockOnComp->DeactivateLockOn();
	else
		LockOnComp->ActivateLockOn();

	EndAbility(Handle, ActorInfo, ActivationInfo, false,false);
}

void UKOLockOnGameplayAbility::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility,bWasCancelled);
}


