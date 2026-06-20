#include "KOGA_BossGroggyBase.h"

#include "AIController.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Enemy/Boss/KOAIC_BossChapter01.h"
#include "Character/Enemy/Boss/KOBossBase.h"

UKOGA_BossGroggyBase::UKOGA_BossGroggyBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
 
	// 그로기 중 재발동 방지
	ActivationOwnedTags.AddTag(KOGameplayTags::State_Boss_Groggy);
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Boss_Groggy);
}

void UKOGA_BossGroggyBase::ActivateAbility(
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
 
	// 보스 그로기 진입
	AKOBossBase* Boss = Cast<AKOBossBase>(GetAvatarCharacter());
	if (!Boss)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
 
	Boss->OnGroggyBegin();
 
	UE_LOG(LogTemp, Log, TEXT("[BossGroggy] 그로기 진입 / 유지시간 : %.1f초"), GroggyDuration);
 
	// 그로기 유지 타이머
	GetWorld()->GetTimerManager().SetTimer(
		GroggyTimerHandle,
		this,
		&UKOGA_BossGroggyBase::OnGroggyTimerEnd,
		GroggyDuration,
		false
	);
}

void UKOGA_BossGroggyBase::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, 
	bool bWasCancelled)
{
	// 타이머 정리
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(GroggyTimerHandle);
	}
	
	// 보스 그로기 종료
	if (AKOBossBase* Boss = Cast<AKOBossBase>(GetAvatarCharacter()))
	{
		Boss->OnGroggyEnd();
		UE_LOG(LogTemp, Log, TEXT("[BossGroggy] 그로기 종료"));
	}
 
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UKOGA_BossGroggyBase::OnGroggyTimerEnd()
{
	UE_LOG(LogTemp, Log, TEXT("[BossGroggy] 그로기 타이머 종료"));
	
	ACharacter* Character = GetAvatarCharacter();
	if (Character)
	{
		AAIController* AIC = Cast<AAIController>(Character->GetController());
		if (AIC)
		{
			if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
			{
				BB->SetValueAsBool(AKOAIC_BossChapter01::bIsGroggyKey, false);
			}
		}
	}
 
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
