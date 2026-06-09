#include "KOGA_Attack_Heavy.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayEffect.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"

UKOGA_Attack_Heavy::UKOGA_Attack_Heavy()
{
}

void UKOGA_Attack_Heavy::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!TriggerEventData)
	{
		UE_LOG(LogTemp, Error, TEXT("AttackHeavy 이벤트 태그 없이 실행"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo) || !TriggerEventData)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	const UObject* RawMontageObj = TriggerEventData->OptionalObject;
	UAnimMontage* MontageToPlay = Cast<UAnimMontage>(const_cast<UObject*>(RawMontageObj));
	
	const UObject* RawEffectObj = TriggerEventData->OptionalObject2;
	DamageEffectClass = Cast<UClass>(const_cast<UObject*>(RawEffectObj));

	if (!MontageToPlay)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	UAbilityTask_WaitGameplayEvent* HitTask = 
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Hit);
	HitTask->EventReceived.AddDynamic(this, &UKOGA_Attack_Heavy::OnHitEventReceived);
	HitTask->ReadyForActivation();
	
	UAbilityTask_PlayMontageAndWait* MontageTask = 
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		MontageToPlay,
		1.0f,
		NAME_None,
		false
	);
	
	MontageTask->OnCompleted.AddDynamic(this, &UKOGA_Attack_Heavy::OnMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(this, &UKOGA_Attack_Heavy::OnMontageEnded);
	MontageTask->OnCancelled.AddDynamic(this, &UKOGA_Attack_Heavy::OnMontageEnded);
	MontageTask->OnBlendOut.AddDynamic(this, &UKOGA_Attack_Heavy::OnMontageEnded);
	MontageTask->ReadyForActivation();
}

void UKOGA_Attack_Heavy::OnMontageEnded()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UKOGA_Attack_Heavy::OnHitEventReceived(FGameplayEventData Payload)
{
	const AActor* TargetActor = Payload.Target;
	if (!TargetActor || !DamageEffectClass)
	{
		return;
	}
	
	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor);
	if (TargetASC)
	{
		FGameplayEffectContextHandle Context = CurrentActorInfo->AbilitySystemComponent->MakeEffectContext();
		Context.AddInstigator(GetAvatarActorFromActorInfo(), GetAvatarActorFromActorInfo());
		
		if (Payload.TargetData.Num() > 0)
		{
			const FHitResult* HitResult = Payload.TargetData.Get(0)->GetHitResult();
			if (HitResult)
			{
				Context.AddHitResult(*HitResult, true);
			}
		}
		
		FGameplayEffectSpecHandle Spec = 
			CurrentActorInfo->AbilitySystemComponent->MakeOutgoingSpec(
				DamageEffectClass,
				GetAbilityLevel(),
				Context
			);
		
		if (Spec.IsValid())
		{
			CurrentActorInfo->AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(
				*Spec.Data.Get(),
				TargetASC
			);
		}
	}
}
