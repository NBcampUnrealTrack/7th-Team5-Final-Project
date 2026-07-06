#include "KOGA_HitReact.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/Ability/AbilityTask/AbilityTask_HitStop.h"
#include "AbilitySystem/Attribute/KOGroggySet.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Character/Enemy/KOBaseEnemy.h"
#include "GameFramework/Character.h"

UKOGA_HitReact::UKOGA_HitReact()
{
	InstancingPolicy  = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = KOGameplayTags::Event_HitReact; 
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
	
	ActivationOwnedTags.AddTag(KOGameplayTags::State_Character_HitReacting);
	
	// 사망 / 무적 / 슈퍼아머 / 패리그로기 중 진입차단 
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Character_Dead);
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Character_Guard_Blocking);
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Character_Invincible);
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Character_SuperArmor);
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Enemy_Parried);
}

void UKOGA_HitReact::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	// 1. Commit Ability 
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	// 에너미가 Groggy가 0이면 즉시 종료
	UAbilitySystemComponent* ASC=GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return;
	}
	
	if (const UKOGroggySet* GroggySet=ASC->GetSet<UKOGroggySet>())
	{
		if (GroggySet->GetGroggyHealth()==0.f)
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}
	}
	
	
	// 2. Caching 
	if (TriggerEventData) CachedTriggerEventData = *TriggerEventData;
	
	// 3. Select Direction For Montage 
	const FGameplayTagContainer& Tags = CachedTriggerEventData.InstigatorTags;
	if (Tags.HasTag(KOGameplayTags::Event_HitReact_Backward)) HitDirection = EHitDirection::Backward;
	else if (Tags.HasTag(KOGameplayTags::Event_HitReact_Left)) HitDirection = EHitDirection::Left;
	else if (Tags.HasTag(KOGameplayTags::Event_HitReact_Right)) HitDirection = EHitDirection::Right;	
	else HitDirection = EHitDirection::Forward;
	
	//Enemy일 경우 Hit 브로드캐스트
	if (AKOBaseEnemy* Enemy= Cast<AKOBaseEnemy>(GetAvatarCharacter()))
	{
		Enemy->OnHitEvent.ExecuteIfBound(true);	
	}
	
	// 4. Hit Stop Task  
	UAbilityTask_HitStop* HitStopTask = 
		UAbilityTask_HitStop::HitStop(
			this, 
			 HitStopDuration,
			 HitStopTimeDilation,
			 bAffectInstigator
		);
	
	HitStopTask->OnFinished.AddDynamic(this, &ThisClass::OnHitStopFinished);
	HitStopTask->ReadyForActivation();
	
	
	// 5. Gameplay Cue 
	FGameplayCueParameters CueParams; 
	if (CachedTriggerEventData.ContextHandle.IsValid())
	{
		if (const FHitResult* HitResult = CachedTriggerEventData.ContextHandle.GetHitResult())
		{
			CueParams.Location = HitResult->ImpactPoint;
			CueParams.Normal = HitResult->ImpactNormal;
		}
	}
	CueParams.RawMagnitude = CachedTriggerEventData.EventMagnitude;
	GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(
		KOGameplayTags::GameplayCue_HitImpact, CueParams);
	
}

void UKOGA_HitReact::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	//Enemy일 경우 Hit 브로드캐스트
	if (AKOBaseEnemy* Enemy= Cast<AKOBaseEnemy>(GetAvatarCharacter()))
	{
		Enemy->OnHitEvent.ExecuteIfBound(false);
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UKOGA_HitReact::ExecuteKnockBack(const FGameplayEventData& EventData)
{
	// FVector LaunchDir =
	// 	EventData.ContextHandle.GetHitResult() ? EventData.ContextHandle.GetHitResult()->ImpactNormal* -1.f :
	// 	GetAvatarCharacter() ? GetAvatarCharacter()->GetActorForwardVector() *-1 : 
	// 	FVector(0, 0, 0);
	//
	//
	// const bool bIsLaunch = 
	// 	EventData.InstigatorTags.HasTag(KOGameplayTags::Event_HitReact_KnockBack_Launch);
	// if (bIsLaunch) LaunchDir.Z = 0.8f;
	//
	// LaunchDir.Normalize();
	//
	// ACharacter* Character = GetAvatarCharacter();
	// if (!Character) return;
	//
	// Character->LaunchCharacter(
	// 	LaunchDir * EventData.EventMagnitude,
	// 	true,
	// 	bIsLaunch
	// );
	
	ACharacter* Character = GetAvatarCharacter();
	
	FVector LaunchDir = FVector::ZeroVector;
	
	if (const AActor* Attacker = EventData.Instigator.Get())
	{
		LaunchDir = Character->GetActorLocation() - Attacker->GetActorLocation();
		LaunchDir.Z = 0.0f;
	}
	else if (EventData.ContextHandle.GetHitResult())
	{
		LaunchDir = EventData.ContextHandle.GetHitResult()->ImpactPoint * -1.0f;
		LaunchDir.Z = 0.0f;
	}
	else
	{
		LaunchDir = Character->GetActorForwardVector() * -1.0f;
	}
	
	LaunchDir.Normalize();
	
	// 기존 공중으로 뛰우는 공격 로직
	const bool bIsLaunch = EventData.InstigatorTags.HasTag(KOGameplayTags::Event_HitReact_KnockBack_Launch);
	
	if (bIsLaunch)
	{
		LaunchDir.Z = 0.0f;
		LaunchDir.Normalize();
	}
	
	float ActualKnockBack = (KnockBackAmount > 0.f) ? KnockBackAmount : EventData.EventMagnitude;
	
	Character->LaunchCharacter(
		LaunchDir * ActualKnockBack,
		true,
		bIsLaunch
	);
}

void UKOGA_HitReact::RotateTowardsAttacker(const FGameplayEventData& EventData)
{
	ACharacter* Character = GetAvatarCharacter();
	if (!Character) return;
	
	FVector TargetLocation = FVector::ZeroVector;
	
	if (const AActor* Attacker = EventData.Instigator.Get())
	{
		TargetLocation = Attacker->GetActorLocation();
	}
	else if (EventData.ContextHandle.GetHitResult())
	{
		FVector ImpactNormal = EventData.ContextHandle.GetHitResult()->ImpactNormal;
		TargetLocation = Character->GetActorLocation() + (ImpactNormal * 100.0f);
	}
	else
	{
		return;
	}
	
	FVector DirectionToTarget = TargetLocation - Character->GetActorLocation();
	DirectionToTarget.Z = 0.0f;
	
	if (!DirectionToTarget.IsNearlyZero())
	{
		FRotator NewRotation = DirectionToTarget.Rotation();
		Character->SetActorRotation(NewRotation);
	}
}

void UKOGA_HitReact::OnHitStopFinished()
{
	RotateTowardsAttacker(CachedTriggerEventData);
	ExecuteKnockBack(CachedTriggerEventData);
	
	UAnimMontage* Montage = DirectionalMontage.FindRef(HitDirection);
	if (!Montage)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	
	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			Montage,
			1.0f,
			NAME_None,
			false
		);
	
	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCancelled);
	MontageTask->ReadyForActivation();
	
} 

void UKOGA_HitReact::OnMontageCompleted()
{
	if (GE_Invincible)
	{
		ApplyEffectSetByCallerToSelf(
			GE_Invincible,
			KOGameplayTags::Data_DebuffTime,
			InvincibleDuration
		);
	}
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UKOGA_HitReact::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
