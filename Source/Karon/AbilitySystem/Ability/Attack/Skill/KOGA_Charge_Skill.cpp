#include "KOGA_Charge_Skill.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "AbilitySystem/Ability/AbilityTask/AbilityTask_Tick.h"
#include "AbilitySystem/Effect/KOGameplayEffectContext.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Data/KO_HitData.h"

UKOGA_Charge_Skill::UKOGA_Charge_Skill()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	SetAssetTags(FGameplayTagContainer(KOGameplayTags::Input_Ability_Skill_DimensionSever));
	
	MaxChargeTime = 2.0f;
    ChargeTickRate = 0.05f;
    CurrentChargeTime = 0.0f;
    bIsCharging = false;
}

void UKOGA_Charge_Skill::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
    
    if (!IsActive()) return; 
    
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
       EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
       return;
    }
    
    if (ChargeMontage)
    {
       ChargeMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
          this, NAME_None, ChargeMontage, 1.0f, NAME_None, true
       );
       
       if (ChargeMontageTask)
       {
          ChargeMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnChargeMontageInterrupted);
          ChargeMontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnChargeMontageInterrupted);
          ChargeMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnChargeMontageInterrupted);
          ChargeMontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnChargeMontageInterrupted);
          ChargeMontageTask->ReadyForActivation();
       }
    }
    else
    {
       UE_LOG(LogTemp, Warning, TEXT("[%s] ChargeMontage가 없음"), *GetName());
    }
    
    StartCharging();
}

void UKOGA_Charge_Skill::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility, bool bWasCancelled)
{
    StopCharging();
    
    if (TickTask)
    {
       TickTask->StopTask();
       TickTask = nullptr;
    }
    
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UKOGA_Charge_Skill::InputReleased(
       const FGameplayAbilitySpecHandle Handle,
       const FGameplayAbilityActorInfo* ActorInfo,
       const FGameplayAbilityActivationInfo ActivationInfo)
{
    if (bIsCharging)
    {
       StopCharging();
       
       float ChargePercentage = FMath::Clamp(CurrentChargeTime / MaxChargeTime, 0.0f, 1.0f);
       UE_LOG(LogTemp, Warning, TEXT("입력 해제. 차징 시간: %f (비율: %.1f%%)"), CurrentChargeTime, ChargePercentage * 100.0f);
       
       ExecuteAttack(CurrentChargeTime);
    }
}

void UKOGA_Charge_Skill::ExecuteAttack(float ChargeTime)
{
    if (ChargeDamageMultiplierCurve)
    {
        CurrentDamageMultiplier = ChargeDamageMultiplierCurve->GetFloatValue(ChargeTime);
    }
    else
    {
        CurrentDamageMultiplier = 1.0f;
    }
    
    if (!AttackMontageData.Montage)
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }

    UAbilityTask_WaitGameplayEvent* TraceStartTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Trace_Start);
    if (TraceStartTask)
    {
        TraceStartTask->EventReceived.AddDynamic(this, &ThisClass::OnTraceStart);
        TraceStartTask->ReadyForActivation();
    }

    UAbilityTask_WaitGameplayEvent* TraceEndTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Trace_End);
    if (TraceEndTask)
    {
        TraceEndTask->EventReceived.AddDynamic(this, &ThisClass::OnTraceEnd);
        TraceEndTask->ReadyForActivation();
    }

    AttackMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this, NAME_None, AttackMontageData.Montage, AttackMontageData.PlayRate
    );

    if (AttackMontageTask)
    {
        AttackMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnAttackMontageCompleted);
        AttackMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnAttackMontageInterrupted);
        AttackMontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnAttackMontageInterrupted);
        AttackMontageTask->ReadyForActivation();
    }
}

void UKOGA_Charge_Skill::OnChargeMontageInterrupted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UKOGA_Charge_Skill::OnAttackMontageCompleted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UKOGA_Charge_Skill::OnAttackMontageInterrupted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UKOGA_Charge_Skill::OnChargeTick()
{
    if (!bIsCharging) return;
    
    CurrentChargeTime += ChargeTickRate;
    
    if (CurrentChargeTime >= MaxChargeTime)
    {
       CurrentChargeTime = MaxChargeTime;
       
       UWorld* World = GetWorld();
       if (World)
       {
          World->GetTimerManager().ClearTimer(ChargeTimerHandle);
       }
    }
}

void UKOGA_Charge_Skill::StartCharging()
{
    CurrentChargeTime = 0.0f;
    bIsCharging = true;
    
    UWorld* World = GetWorld();
    if (World)
    {
       World->GetTimerManager().SetTimer(
          ChargeTimerHandle, this, &ThisClass::OnChargeTick, ChargeTickRate, true
       );
    }
}

void UKOGA_Charge_Skill::StopCharging()
{
    bIsCharging = false;
    
    UWorld* World = GetWorld();
    if (World)
    {
       World->GetTimerManager().ClearTimer(ChargeTimerHandle);
    }
    
    if (ChargeMontageTask)
    {
       ChargeMontageTask->EndTask();
       ChargeMontageTask = nullptr;
    }
}

void UKOGA_Charge_Skill::OnTraceStart(FGameplayEventData Payload)
{
    if (const UKO_HitData* ReceivedData = Cast<UKO_HitData>(Payload.OptionalObject))
    {
       CachedHitData = ReceivedData;
       HitStopDuration = ReceivedData->HitData.HitStopDuration;
       HitStopTimeDilation = ReceivedData->HitData.HitStopTimeDilation;
    }

    ResetHitActors();
    TraceData.bIsFirstTick = true;

    TickTask = UAbilityTask_Tick::CreateTickTask(this);
    if (TickTask)
    {
       TickTask->OnTick.AddDynamic(this, &ThisClass::PerformWeaponTrace);
       TickTask->ReadyForActivation();
    }
}

void UKOGA_Charge_Skill::OnTraceEnd(FGameplayEventData Payload)
{
    if (TickTask)
    {
       TickTask->StopTask();
       TickTask = nullptr;
    }
}

