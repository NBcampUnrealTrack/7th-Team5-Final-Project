#include "KOGA_HitReact.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/Ability/AbilityTask/AbilityTask_HitStop.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "GameFramework/Character.h"

UKOGA_HitReact::UKOGA_HitReact()
{
	InstancingPolicy  = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = KOGameplayTags::Event_HitReact; 
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
	
	// 사망 / 무적 중 진입차단 
	// ActivationBlockedTags.AddTag(KOGameplayTags::State_Character_Dead);
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Character_Invincible);
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
	
	// 2. Caching 
	if (TriggerEventData) CachedTriggerEventData = *TriggerEventData;
	
	// 3. Select Direction For Monatge 
	const FGameplayTagContainer& Tags = CachedTriggerEventData.InstigatorTags;
	if (Tags.HasTag(KOGameplayTags::Event_HitReact_Backward)) HitDirection = EHitDirection::Backward;
	else if (Tags.HasTag(KOGameplayTags::Event_HitReact_Left)) HitDirection = EHitDirection::Left;
	else if (Tags.HasTag(KOGameplayTags::Event_HitReact_Right)) HitDirection = EHitDirection::Right;	
	else HitDirection = EHitDirection::Forward;
	
	// 4. Hit Stop Task  
	UAbilityTask_HitStop* HitStopTask = 
		UAbilityTask_HitStop::HitStop(
			this, 
			 HitStopDuration,
			 HitStopTimeDilation,
			 bAffectInstigator
		);
	
	if (HitStopTask)
	{
		HitStopTask->OnFinished.AddDynamic(this, &ThisClass::OnHitStopFinished);
		HitStopTask->ReadyForActivation();
	}
	
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

void UKOGA_HitReact::ExecuteKnockBack(const FGameplayEventData& EventData)
{
	FVector LaunchDir =
		EventData.ContextHandle.GetHitResult() ? EventData.ContextHandle.GetHitResult()->ImpactNormal* -1.f :
		GetAvatarCharacter() ? GetAvatarCharacter()->GetActorForwardVector() *-1 : 
		FVector(0, 0, 0);
	
	
	const bool bIsLaunch = 
		EventData.InstigatorTags.HasTag(KOGameplayTags::Event_HitReact_KnockBack_Launch);
	if (bIsLaunch) LaunchDir.Z = 0.8f;
	
	LaunchDir.Normalize();
	
	ACharacter* Character = GetAvatarCharacter();
	if (!Character) return;
	
	Character->LaunchCharacter(
		LaunchDir * EventData.EventMagnitude,
		true,
		bIsLaunch
	);
}

void UKOGA_HitReact::OnHitStopFinished()
{
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
	
	if (MontageTask)
	{
		MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
		MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);
		MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCancelled);
		MontageTask->ReadyForActivation();
	}
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
