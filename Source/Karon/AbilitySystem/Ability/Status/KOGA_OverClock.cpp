#include "KOGA_OverClock.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "Utility/Log/KOLogManager.h"

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
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return; 
	}
	
	 ClockGainTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
	 	this, KOGameplayTags::Event_Clock_Gain);
	
	ClockGainTask->EventReceived.AddDynamic(this, &ThisClass::OnClockGain); 
	ClockGainTask->ReadyForActivation(); 
	
	
	OverClockStartTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, KOGameplayTags::Event_OverClock_Start);
	
	OverClockStartTask->EventReceived.AddDynamic(this, &ThisClass::OnOverClockStart); 
	OverClockStartTask->ReadyForActivation(); 
	
	 OverClockEndTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
	 	this, KOGameplayTags::Event_OverClock_End);
	
	OverClockEndTask->EventReceived.AddDynamic(this, &ThisClass::OnOverClockEnd); 
	OverClockEndTask->ReadyForActivation(); 
}

void UKOGA_OverClock::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (UAbilitySystemComponent* ASC = GetASC())
	{
		ASC->RemoveActiveGameplayEffect(BuffEffectHandle);
		ASC->RemoveActiveGameplayEffect(DrainEffectHandle);
	}
	
	GetWorld()->GetTimerManager().ClearTimer(SlowMotionTimerHandle);
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UKOGA_OverClock::OnClockGain(FGameplayEventData Payload)
{
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC || !GE_ClockGain || Payload.EventMagnitude <= 0.f) return;
	
	const float ClockAmount = 
		ASC->HasMatchingGameplayTag(KOGameplayTags::State_Character_OverClock) ?
		Payload.EventMagnitude : Payload.EventMagnitude / 2.5;
	
	ApplyEffectSetByCallerToSelf(
		GE_ClockGain,
		KOGameplayTags::Data_Attribute_Combat_Clock,
		ClockAmount
		,1.f
	);
}

void UKOGA_OverClock::OnOverClockStart(FGameplayEventData Payload)
{
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC) return; 
	
	KO_LOG(Combat, Log, TEXT("OverClock Start")); 
	
	ASC->AddLooseGameplayTag(KOGameplayTags::State_Character_OverClock);
	
	if (GE_OverClockBuff) 
		BuffEffectHandle = ApplyEffectToSelf(GE_OverClockBuff, 1.f);
	 
	if (GE_ClockDrain) 
		DrainEffectHandle = ApplyEffectToSelf(GE_ClockDrain, 1.f);
	

	if (ActivationCueTag.IsValid())
	{
		FGameplayCueParameters CueParams;
		CueParams.Instigator = GetAvatarActorFromActorInfo();
		CueParams.EffectContext = ASC->MakeEffectContext();
		
		ASC->ExecuteGameplayCue(ActivationCueTag, CueParams);
	}
	
	// TODO: Cue로 이전 
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.2f);
	
	GetWorld()->GetTimerManager().SetTimer(
		SlowMotionTimerHandle,
		this,
		&ThisClass::RestoreTimeDilation,
		0.2f,
		false
	);
}

void UKOGA_OverClock::OnOverClockEnd(FGameplayEventData Payload)
{
	KO_LOG(Combat, Log, TEXT("OverClock End")); 
	if (UAbilitySystemComponent* ASC = GetASC())
	{
		ASC->RemoveActiveGameplayEffect(BuffEffectHandle);
		ASC->RemoveActiveGameplayEffect(DrainEffectHandle);
		
		ASC->SetLooseGameplayTagCount(KOGameplayTags::State_Character_OverClock, 0);
	}
}


void UKOGA_OverClock::RestoreTimeDilation()
{
	// TODO: Cue로 이전 
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
}
