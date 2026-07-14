#include "KOGA_HitReact.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/Ability/AbilityTask/AbilityTask_HitStop.h"
#include "AbilitySystem/Attribute/KOGroggySet.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Character/Enemy/KOBaseEnemy.h"
#include "Data/KO_HitData.h"
#include "Utility/Log/KOLogManager.h"
#include "GameFramework/Character.h"

UKOGA_HitReact::UKOGA_HitReact()
{
	InstancingPolicy  = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	bRetriggerInstancedAbility = true;
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
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (const UKOGroggySet* GroggySet = ASC->GetSet<UKOGroggySet>())
	{
		if (GroggySet->GetGroggyHealth()==0.f)
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}
	}
	
	ASC->CancelAbilities(&CancelTags, nullptr, this);
	
	// 2. Caching 
	if (TriggerEventData) CachedTriggerEventData = *TriggerEventData;
	
	// 3. Select Direction For Montage 
	HitDirection = EHitDirection::Forward; // 기본값
  
	if (ACharacter* Defender = Cast<ACharacter>(GetAvatarCharacter()))
	{
		FVector DefenderLoc = Defender->GetActorLocation();
		DefenderLoc.Z = 0.0f;
    
		FVector HitOrigin = FVector::ZeroVector;
		bool bFoundOrigin = false;

		if (CachedTriggerEventData.ContextHandle.IsValid() && CachedTriggerEventData.ContextHandle.GetHitResult())
		{
			HitOrigin = CachedTriggerEventData.ContextHandle.GetHitResult()->ImpactPoint;
			bFoundOrigin = true;
		}
		else if (CachedTriggerEventData.ContextHandle.IsValid() && CachedTriggerEventData.ContextHandle.GetEffectCauser())
		{
			HitOrigin = CachedTriggerEventData.ContextHandle.GetEffectCauser()->GetActorLocation();
			bFoundOrigin = true;
		}
		else if (const AActor* Attacker = CachedTriggerEventData.Instigator.Get())
		{
			HitOrigin = Attacker->GetActorLocation();
			bFoundOrigin = true;
		}
		
		if (bFoundOrigin)
		{
			HitOrigin.Z = 0.0f;
			FVector DirToHit = (HitOrigin - DefenderLoc).GetSafeNormal();

			FVector DefForward = Defender->GetActorForwardVector();
			FVector DefRight = Defender->GetActorRightVector();
			DefForward.Z = 0.0f; 
			DefRight.Z = 0.0f; 
			DefForward.Normalize();
			DefRight.Normalize();
        
			float ForwardDot = FVector::DotProduct(DirToHit, DefForward);
			float RightDot = FVector::DotProduct(DirToHit, DefRight);
        
			if (FMath::Abs(ForwardDot) >= FMath::Abs(RightDot))
			{
				HitDirection = (ForwardDot > 0.0f) ? EHitDirection::Forward : EHitDirection::Backward;
			}
			else
			{
				HitDirection = (RightDot > 0.0f) ? EHitDirection::Right : EHitDirection::Left;
			}
		}
	}
	
	bool bRequireRotation = false;
	
	UAnimMontage* Montage = DirectionalMontage.FindRef(HitDirection);

	if (!Montage || DirectionalMontage.Num() <= 1)
	{
		HitDirection = EHitDirection::Forward;
		Montage = DirectionalMontage.FindRef(EHitDirection::Forward);
		
		bRequireRotation = true;
	}
	
	if (!Montage)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	//Enemy일 경우 Hit 브로드캐스트
	if (AKOBaseEnemy* Enemy= Cast<AKOBaseEnemy>(GetAvatarCharacter()))
	{
		Enemy->OnHitEvent.ExecuteIfBound(true);	
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
	ApplyGameplayCues(CueTags, CueParams);
	
	if (bRequireRotation)
	{
		RotateTowardsAttacker(CachedTriggerEventData);	
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
    
	// 몽타주 재생 시작
	MontageTask->ReadyForActivation();
	
	ExecuteKnockBack(CachedTriggerEventData);
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
	ACharacter* Character = GetAvatarCharacter();
	if (!Character) return; 
	
	float FinalKnockBack = KnockBackAmount; 

	if (const UKO_HitData* ReceivedData = Cast<UKO_HitData>(EventData.OptionalObject))
	{
		FinalKnockBack = ReceivedData->HitData.KnockBackAmount;
		KO_LOG(Combat, Log, TEXT("Apply KnockBack Amount: %f"), FinalKnockBack);
	}
	else if (EventData.EventMagnitude > 0.f)
	{
		FinalKnockBack = EventData.EventMagnitude;
	}
	
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
		LaunchDir.Z = 1.5f;
	}
	else
	{
		LaunchDir.Z = 0.0f;
	}
	
	LaunchDir.Normalize();
	
	Character->LaunchCharacter(
		LaunchDir * FinalKnockBack,
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
