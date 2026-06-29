#include "KOGA_OverClock.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitAttributeChange.h"
#include "AbilitySystem/Attribute/KOCombatSet.h"
#include "Kismet/GameplayStatics.h"

UKOGA_OverClock::UKOGA_OverClock()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UKOGA_OverClock::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC || !OverClockBuffEffectClass || !OverClockDrainEffectClass)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	FGameplayEffectContextHandle BuffContext = ASC->MakeEffectContext();
	BuffContext.AddSourceObject(GetAvatarActorFromActorInfo());
	BuffEffectHandle = 
		ASC->ApplyGameplayEffectToSelf(
			OverClockBuffEffectClass->GetDefaultObject<UGameplayEffect>(),
			1.0f,
			BuffContext
		);
	
	FGameplayEffectContextHandle DrainContext = ASC->MakeEffectContext();
	DrainContext.AddSourceObject(GetAvatarActorFromActorInfo());
	DrainEffectHandle =
		ASC->ApplyGameplayEffectToSelf(
			OverClockDrainEffectClass->GetDefaultObject<UGameplayEffect>(),
			1.0f,
			DrainContext
		);
	
	if (ActivationCueTag.IsValid())
	{
		FGameplayCueParameters CueParams;
		CueParams.Instigator = GetAvatarActorFromActorInfo();
		CueParams.EffectContext = ASC->MakeEffectContext();
		
		ASC->ExecuteGameplayCue(ActivationCueTag, CueParams);
	}
	
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.2f);
	
	GetWorld()->GetTimerManager().SetTimer(
		SlowMotionTimerHandle,
		this,
		&ThisClass::RestoreTimeDelation,
		0.2f,
		false
	);
	
	UAbilityTask_WaitAttributeChange* WaitGaugeChange = 
		UAbilityTask_WaitAttributeChange::WaitForAttributeChangeWithComparison(
			this,
			UKOCombatSet::GetOverClockGaugeAttribute(),
			FGameplayTag(),
			FGameplayTag(),
			EWaitAttributeChangeComparison::LessThanOrEqualTo,
			0.01f,
			false
		);
	
	WaitGaugeChange->OnChange.AddDynamic(this, &ThisClass::OnOverClockGaugeEmpty);
	WaitGaugeChange->ReadyForActivation();
}

void UKOGA_OverClock::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		ASC->RemoveActiveGameplayEffect(BuffEffectHandle);
		ASC->RemoveActiveGameplayEffect(DrainEffectHandle);
	}
}

void UKOGA_OverClock::OnOverClockGaugeEmpty()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UKOGA_OverClock::RestoreTimeDelation()
{
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
}
