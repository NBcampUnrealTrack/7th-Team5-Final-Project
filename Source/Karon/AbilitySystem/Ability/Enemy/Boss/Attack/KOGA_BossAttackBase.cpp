#include "KOGA_BossAttackBase.h"

#include "AbilitySystemInterface.h"
#include "AIController.h"

#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/Attribute/KOCombatSet.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Enemy/Boss/KOAIC_BossChapter01.h"
#include "Character/Enemy/Boss/KOBossBase.h"
#include "Character/Enemy/Boss/KOBossDataAsset.h"

UKOGA_BossAttackBase::UKOGA_BossAttackBase()
{
	ActivationOwnedTags.AddTag(KOGameplayTags::State_Character_Attacking);
 
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Character_Attacking);
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Boss_Groggy);
}
 
void UKOGA_BossAttackBase::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
 
	if (!IsTargetInRange())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!AttackMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
 
	// 몽타주 재생 태스크 생성
	UAbilityTask_PlayMontageAndWait* MontageTask = 
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		AttackMontage,
		MontageSpeed,
		NAME_None,
		false,
		1.0f
	);
	
	MontageTask->OnCompleted.AddDynamic(this, &UKOGA_BossAttackBase::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UKOGA_BossAttackBase::OnMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &UKOGA_BossAttackBase::OnMontageCancelled);
 
	MontageTask->ReadyForActivation();
}

void UKOGA_BossAttackBase::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
 
void UKOGA_BossAttackBase::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

bool UKOGA_BossAttackBase::IsTargetInRange() const
{
	// TODO:
	// 역할 분리 필요. 어빌리티에서 타겟의 위치를 구하는건 맞지않음. 
	// BT에서 처리 해야할듯. 
	
	
	AKOBossBase* Boss = Cast<AKOBossBase>(GetAvatarCharacter()); 
	if (!Boss) return false; 
 
	AAIController* AIC = Cast<AAIController>(Boss->GetController());
	if (!AIC) return false;
	
	UBlackboardComponent* BB = AIC->GetBlackboardComponent();
	if (!BB) return false;
	
	AActor* Target = Cast<AActor>(BB->GetValueAsObject(AKOAIC_BossChapter01::TargetActorKey));
	if (!Target) return false;
	
	// DA에서 GA 클래스 기준으로 AttackRange 조회
	UKOBossDataAsset* DA = Boss->GetDataAsset();
	const float AttackRange = DA ? DA->GetAttackRange(GetClass()) : 300.f;
 
	return FVector::Dist(Boss->GetActorLocation(),Target->GetActorLocation()) <= AttackRange;
}
