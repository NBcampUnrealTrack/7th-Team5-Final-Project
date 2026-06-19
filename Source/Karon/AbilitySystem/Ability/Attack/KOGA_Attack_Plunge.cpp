#include "AbilitySystem/Ability/Attack/KOGA_Attack_Plunge.h"
#include "NativeGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"



UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Ability_Attack_Plunge, "Input.Ability.Attack.Plunge");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Plunge_Land,           "Event.Plunge.Land");

UKOGA_Attack_Plunge::UKOGA_Attack_Plunge()
{
    SetAssetTags(FGameplayTagContainer(TAG_Input_Ability_Attack_Plunge));
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

// ── ActivateAbility ───────────────────────────────────────────────────────
void UKOGA_Attack_Plunge::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
    // 무기 미장착 시 CommitAbility 전에 차단
    if (RequiredWeaponTag.IsValid())
    {
        UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
        if (!ASC || !ASC->HasMatchingGameplayTag(RequiredWeaponTag))
        {
            UE_LOG(LogTemp, Error, TEXT("[Plunge Debug] 무기 태그 조건 미충족으로 종료!")); // 로그 추가
            EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
            return;
        }
    }

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        UE_LOG(LogTemp, Error, TEXT("[Plunge Debug] CommitAbility 검증 실패로 종료!")); // 로그 추가
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    
    ChargeRatio  = 0.f;
    bIsPlunging  = false;
    ChargeStartTime = GetWorld()->GetTimeSeconds();
    
    // 차징(준비) 애니메이션 재생
    if (ChargeMontage)
    {
        CurrentMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, ChargeMontage, 1.0f);
        CurrentMontageTask->OnCancelled.AddDynamic(this, &UKOGA_Attack_Plunge::OnMontageCancelled);
        CurrentMontageTask->OnInterrupted.AddDynamic(this, &UKOGA_Attack_Plunge::OnMontageCancelled);
        CurrentMontageTask->ReadyForActivation();
        UE_LOG(LogTemp, Log, TEXT("[Plunge Debug] 차징 몽타주 태스크 시작됨")); // 로그 추가
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[Plunge Debug] ChargeMontage가 빌어먹게도 Null 상태입니다!")); // 로그 추가
    }
}

// [추가] 키보드에서 손을 떼었을 때 자동으로 호출되는 함수
void UKOGA_Attack_Plunge::InputReleased(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo)
{
    Super::InputReleased(Handle, ActorInfo, ActivationInfo);

    if (bIsPlunging) return;

    // 누르고 있던 시간을 바탕으로 차징 비율 계산
    float TimeHeld = GetWorld()->GetTimeSeconds() - ChargeStartTime;
    ChargeRatio = FMath::Clamp(TimeHeld / MaxChargeTime, 0.0f, 1.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("[Plunge Debug] 키 뗌 감지! 차징 비율: %.2f"), ChargeRatio);
    
    // 하강 단계로 넘어감
    StartPlunge();
}

// ── 페이즈 1 → 2: 하강 ───────────────────────────────────────────────────
void UKOGA_Attack_Plunge::StartPlunge()
{
    bIsPlunging = true;
    
    // [추가] 다음 단계로 넘어가기 전, 차징 몽타주 태스크의 이벤트 바인딩을 제거하고 안전하게 종료합니다.
    if (CurrentMontageTask)
    {
        CurrentMontageTask->OnCancelled.RemoveDynamic(this, &UKOGA_Attack_Plunge::OnMontageCancelled);
        CurrentMontageTask->OnInterrupted.RemoveDynamic(this, &UKOGA_Attack_Plunge::OnMontageCancelled);
        CurrentMontageTask->EndTask();
        CurrentMontageTask = nullptr;
    }

    if (ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
    {
        UCharacterMovementComponent* Move = Character->GetCharacterMovement();
        Move->GravityScale = 0.f;
        Move->Velocity     = FVector(0.f, 0.f, -PlungeSpeed);
    }

    if (!FallMontage) { StartLanding(); return; }

    CurrentMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this, NAME_None, FallMontage, 1.0f);
    CurrentMontageTask->OnCompleted.AddDynamic(this, &UKOGA_Attack_Plunge::OnFallMontageCompleted);
    CurrentMontageTask->OnCancelled.AddDynamic(this, &UKOGA_Attack_Plunge::OnMontageCancelled);
    CurrentMontageTask->OnInterrupted.AddDynamic(this, &UKOGA_Attack_Plunge::OnMontageCancelled);
    CurrentMontageTask->ReadyForActivation();
}

void UKOGA_Attack_Plunge::OnFallMontageCompleted()
{
    StartLanding();
}

// ── 페이즈 3: 착지 ────────────────────────────────────────────────────────
void UKOGA_Attack_Plunge::StartLanding()
{
    // [추가] 착지 단계로 가기 전, 하강 몽타주 태스크의 이벤트 바인딩을 제거하고 안전하게 종료합니다.
    if (CurrentMontageTask)
    {
        CurrentMontageTask->OnCompleted.RemoveDynamic(this, &UKOGA_Attack_Plunge::OnFallMontageCompleted);
        CurrentMontageTask->OnCancelled.RemoveDynamic(this, &UKOGA_Attack_Plunge::OnMontageCancelled);
        CurrentMontageTask->OnInterrupted.RemoveDynamic(this, &UKOGA_Attack_Plunge::OnMontageCancelled);
        CurrentMontageTask->EndTask();
        CurrentMontageTask = nullptr;
    }
    
    if (ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
        Character->GetCharacterMovement()->GravityScale = 1.f;

    if (!LandMontage)
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }

    // 착지 타이밍은 LandMontage의 AnimNotify_GameplayEvent("Event.Plunge.Land") 로 지정
    UAbilityTask_WaitGameplayEvent* WaitEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
        this, TAG_Event_Plunge_Land, nullptr, true, true);
    WaitEvent->EventReceived.AddDynamic(this, &UKOGA_Attack_Plunge::OnLandEventReceived);
    WaitEvent->ReadyForActivation();

    CurrentMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this, NAME_None, LandMontage, 1.0f);
    CurrentMontageTask->OnCompleted.AddDynamic(this, &UKOGA_Attack_Plunge::OnLandMontageCompleted);
    CurrentMontageTask->OnCancelled.AddDynamic(this, &UKOGA_Attack_Plunge::OnMontageCancelled);
    CurrentMontageTask->OnInterrupted.AddDynamic(this, &UKOGA_Attack_Plunge::OnMontageCancelled);
    CurrentMontageTask->ReadyForActivation();
}

void UKOGA_Attack_Plunge::OnLandEventReceived(FGameplayEventData Payload)
{
    float DamageMultiplier = FMath::Lerp(1.f, MaxDamageMultiplier, ChargeRatio);
    UE_LOG(LogTemp, Warning, TEXT("[Plunge] 착지 충격! 차징 %.0f%% / 배율 x%.2f"),
        ChargeRatio * 100.f, DamageMultiplier);
    // AttackBase에서 상속받은 함수로 데미지 적용
    // Payload에 적 정보(Target)가 포함되어 있어야 데미지가 들어갑니다.
    SendAttackEventsToTarget(&Payload);
    ApplyHitEffects(&Payload);
}

void UKOGA_Attack_Plunge::OnLandMontageCompleted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UKOGA_Attack_Plunge::OnMontageCancelled()
{
    UE_LOG(LogTemp, Error, TEXT("[Plunge Debug] 몽타주가 재생되지 못하고 Cancel/Interrupt 되었습니다!")); // 로그 추가
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

// ── EndAbility: 중력 복구 보장 ────────────────────────────────────────────
void UKOGA_Attack_Plunge::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
    if (ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
        Character->GetCharacterMovement()->GravityScale = 1.f;

    bIsPlunging = false;
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}