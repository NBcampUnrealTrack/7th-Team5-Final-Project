#include "KOGA_BossGroggyBase.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Character/Enemy/Boss/KOBossBase.h"

UKOGA_BossGroggyBase::UKOGA_BossGroggyBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	AbilityTags.AddTag(KOGameplayTags::State_Boss_Groggy);
	
	ActivationOwnedTags.AddTag(KOGameplayTags::State_Boss_InGroggy);
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Boss_InGroggy);
}
 
void UKOGA_BossGroggyBase::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	UGameplayAbility::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
 
	if (!IsActive()) return;
 
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	// 그로기 진입 시 모든 공격 GA 즉시 종료
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		UGameplayAbility* GroggyInstance = GetCurrentAbilitySpec() ?
			GetCurrentAbilitySpec()->GetPrimaryInstance() : nullptr;
		ASC->CancelAbilities(nullptr, nullptr, GroggyInstance);
	}
 
	AKOBossBase* Boss = Cast<AKOBossBase>(GetAvatarCharacter());
	if (!Boss)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (GroggyMontage)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask =
			UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
				this, NAME_None, GroggyMontage, 1.f, GroggySection, false);
 
		MontageTask->OnCompleted.AddDynamic(this, &UKOGA_BossGroggyBase::OnMontageCompleted);
		MontageTask->OnCancelled.AddDynamic(this, &UKOGA_BossGroggyBase::OnMontageCancelled);
		MontageTask->OnInterrupted.AddDynamic(this, &UKOGA_BossGroggyBase::OnMontageCancelled);
		MontageTask->ReadyForActivation();
	}
	
	GetWorld()->GetTimerManager().SetTimer(
		GroggyTimerHandle,
		this,
		&UKOGA_BossGroggyBase::OnGroggyTimerEnd,
		GroggyDuration,
		false
	);
}
 
void UKOGA_BossGroggyBase::OnGroggyTimerEnd()
{
	ACharacter* Character = GetAvatarCharacter();
	if (Character && GroggyMontage)
	{
		if (UAnimInstance* AnimInst = Character->GetMesh()->GetAnimInstance())
		{
			AnimInst->Montage_JumpToSection(RestoreSection, GroggyMontage);
		}
	}
}
 
void UKOGA_BossGroggyBase::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
 
void UKOGA_BossGroggyBase::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
 
void UKOGA_BossGroggyBase::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, 
	bool bWasCancelled)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(GroggyTimerHandle);
	}
	
	if (AKOBossBase* Boss = Cast<AKOBossBase>(GetAvatarCharacter()))
	{
		Boss->OnGroggyEnd();
	}
 
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
