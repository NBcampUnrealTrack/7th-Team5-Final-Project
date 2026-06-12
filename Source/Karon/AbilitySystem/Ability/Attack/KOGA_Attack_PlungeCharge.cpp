// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Ability/Attack/KOGA_Attack_PlungeCharge.h"
#include "NativeGameplayTags.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Ability_Attack_Plunge, "Input.Ability.Attack.Plunge");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Plunge_Land,           "Event.Plunge.Land");

UKOGA_Attack_PlungeCharge::UKOGA_Attack_PlungeCharge()
{
    SetAssetTags(FGameplayTagContainer(TAG_Input_Ability_Attack_Plunge));
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}


// 진입점

void UKOGA_Attack_PlungeCharge::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData
    )
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // 상태 초기화
    ChargeRatio = 0.f;
    bIsPlunging = false;
    ChargeStartTime = GetWorld()->GetTimeSeconds();

    StartCharge();
}


// 페이즈 1: 차징

void UKOGA_Attack_PlungeCharge::StartCharge()
{
    if (!ChargeMontage)
    {
        // 차징 애니메이션 없어도 바로 하강 가능하게 처리
        StartPlunge();
        return;
    }

    // 차징 몽타주 재생
    CurrentMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this, NAME_None, ChargeMontage, 1.0f
    );
    CurrentMontageTask->OnCompleted.AddDynamic(this, &UKOGA_Attack_PlungeCharge::OnChargeMontageCompleted);
    CurrentMontageTask->OnCancelled.AddDynamic(this, &UKOGA_Attack_PlungeCharge::OnMontageCancelled);
    CurrentMontageTask->OnInterrupted.AddDynamic(this, &UKOGA_Attack_PlungeCharge::OnMontageCancelled);
    CurrentMontageTask->ReadyForActivation();

    // 버튼을 뗐을 때 감지하는 태스크
    UAbilityTask_WaitInputRelease* ReleaseTask =
        UAbilityTask_WaitInputRelease::WaitInputRelease(this, true);
    ReleaseTask->OnRelease.AddDynamic(this, &UKOGA_Attack_PlungeCharge::OnInputReleased);
    ReleaseTask->ReadyForActivation();
}

// 버튼 뗌 → 차징 시간 계산 후 하강
void UKOGA_Attack_PlungeCharge::OnInputReleased(float TimeHeld)
{
    // 차징 비율 계산 (0.0 ~ 1.0)
    ChargeRatio = FMath::Clamp(TimeHeld / MaxChargeTime, 0.f, 1.f);
    UE_LOG(LogTemp, Warning, TEXT("차징 비율: %.2f"), ChargeRatio);

    StartPlunge();
}

void UKOGA_Attack_PlungeCharge::OnChargeMontageCompleted()
{
    // 차징 몽타주가 끝까지 재생됨 = 풀차징
    ChargeRatio = 1.0f;
    StartPlunge();
}


// 페이즈 2: 하강

void UKOGA_Attack_PlungeCharge::StartPlunge()
{
    bIsPlunging = true;

    // 캐릭터 이동 컴포넌트를 꺼내서 중력 무시 + 하강 속도 직접 설정
    ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (Character)
    {
        UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
        Movement->GravityScale = 0.f;           // 중력 끔
        Movement->Velocity = FVector(0, 0, -PlungeSpeed); // 수직 낙하
    }

    if (!FallMontage)
    {
        // 하강 애니 없으면 착지 이벤트를 WaitGameplayEvent로 기다림
        // (AttackBase에서 이미 "Event" 태그 구독 중이므로 OnGameplayEventReceived에서 처리)
        return;
    }

    CurrentMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this, NAME_None, FallMontage, 1.0f
    );
    CurrentMontageTask->OnCompleted.AddDynamic(this, &UKOGA_Attack_PlungeCharge::OnFallMontageCompleted);
    CurrentMontageTask->OnCancelled.AddDynamic(this, &UKOGA_Attack_PlungeCharge::OnMontageCancelled);
    CurrentMontageTask->OnInterrupted.AddDynamic(this, &UKOGA_Attack_PlungeCharge::OnMontageCancelled);
    CurrentMontageTask->ReadyForActivation();
}

void UKOGA_Attack_PlungeCharge::OnFallMontageCompleted()
{
    StartLanding();
}


// 페이즈 3: 착지 충격

void UKOGA_Attack_PlungeCharge::StartLanding()
{
    // 중력 복구
    ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (Character)
    {
        Character->GetCharacterMovement()->GravityScale = 1.f;
    }

    if (!LandMontage)
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }

    CurrentMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this, NAME_None, LandMontage, 1.0f
    );
    CurrentMontageTask->OnCompleted.AddDynamic(this, &UKOGA_Attack_PlungeCharge::OnLandMontageCompleted);
    CurrentMontageTask->OnCancelled.AddDynamic(this, &UKOGA_Attack_PlungeCharge::OnMontageCancelled);
    CurrentMontageTask->OnInterrupted.AddDynamic(this, &UKOGA_Attack_PlungeCharge::OnMontageCancelled);
    CurrentMontageTask->ReadyForActivation();
    // 착지 애니에 AnimNotify_GameplayEvent("Event.Plunge.Land") 추가해놔야
    // OnGameplayEventReceived에서 피해 처리가 됨
}


// 이벤트 수신 (착지 Notify → 피해 처리)

void UKOGA_Attack_PlungeCharge::OnGameplayEventReceived(FGameplayEventData Payload)
{
    if (Payload.EventTag == TAG_Event_Plunge_Land)
    {
        // 차징 비율에 따라 데미지 배율 SetByCaller로 전달
        // GE 안에서 SetByCaller 태그로 배율을 읽어서 최종 데미지 계산
        // (GE 설정 방법은 별도 설명)
        UE_LOG(LogTemp, Warning, TEXT("착지 충격! 차징 비율: %.2f, 배율: %.2f"),
            ChargeRatio, FMath::Lerp(1.f, MaxDamageMultiplier, ChargeRatio));

        Super::OnGameplayEventReceived(Payload); // Base의 GE 적용 호출
        return;
    }
}


// 정리

void UKOGA_Attack_PlungeCharge::OnLandMontageCompleted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UKOGA_Attack_PlungeCharge::OnMontageCancelled()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UKOGA_Attack_PlungeCharge::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
    // 중력 반드시 복구 (어떤 경로로 끝나든)
    ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (Character)
    {
        Character->GetCharacterMovement()->GravityScale = 1.f;
    }

    bIsPlunging = false;
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}