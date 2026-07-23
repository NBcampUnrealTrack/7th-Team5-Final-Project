#include "AbilitySystem/Ability/Attack/KOGA_Attack_Plunge.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/OverlapResult.h"

UKOGA_Attack_Plunge::UKOGA_Attack_Plunge()
{
    SetAssetTags(FGameplayTagContainer(KOGameplayTags::Input_Ability_Skill_Plunge));
    
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
    if (!ChargeMontage) { return; }

    CurrentMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this, NAME_None, ChargeMontage, 1.0f);
    CurrentMontageTask->OnCancelled.AddDynamic(this, &UKOGA_Attack_Plunge::OnMontageCancelled);
    CurrentMontageTask->OnInterrupted.AddDynamic(this, &UKOGA_Attack_Plunge::OnMontageCancelled);
    CurrentMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
    CurrentMontageTask->ReadyForActivation();
}

// [추가] 키보드에서 손을 떼었을 때 자동으로 호출되는 함수
void UKOGA_Attack_Plunge::InputReleased(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo)
{
    Super::InputReleased(Handle, ActorInfo, ActivationInfo);

    if (bIsPlunging) { return; }

    // 누르고 있던 시간을 바탕으로 차징 비율 계산
    const float TimeHeld = GetWorld()->GetTimeSeconds() - ChargeStartTime;
    ChargeRatio = FMath::Clamp(TimeHeld / MaxChargeTime, 0.0f, 1.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("[Plunge Debug] 키 뗌 감지! 차징 비율: %.2f"), ChargeRatio);
    
    // 하강 단계로 넘어감
    StartPlunge();
}

// ── 페이즈 1 → 2: 하강 ───────────────────────────────────────────────────
void UKOGA_Attack_Plunge::StartPlunge()
{
    bIsPlunging = true;
    StopCurrentMontageTask();
    // [추가] 다음 단계로 넘어가기 전, 차징 몽타주 태스크의 이벤트 바인딩을 제거하고 안전하게 종료합니다.
    if (ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
    {
        UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
        Movement->GravityScale = 0.f;
        Movement->Velocity     = FVector(0.f, 0.f, -PlungeSpeed);
    }

    if (!FallMontage) { StartLanding(); return; }

    UAbilityTask_WaitGameplayEvent* WaitEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
        this, KOGameplayTags::Event_Plunge_Land, nullptr, true, true);
    WaitEvent->EventReceived.AddDynamic(this, &UKOGA_Attack_Plunge::OnLandEventReceived);
    WaitEvent->ReadyForActivation();
    
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
    StopCurrentMontageTask();
    // [추가] 착지 단계로 가기 전, 하강 몽타주 태스크의 이벤트 바인딩을 제거하고 안전하게 종료합니다.
    if (ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
    {
        Character->GetCharacterMovement()->GravityScale = 1.f;
    }

    if (!LandMontage)
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }

    // AnimNotify "Event.Plunge.Land" 가 발동되는 순간 범위 피해 적용
    UAbilityTask_WaitGameplayEvent* WaitLandEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
        this, KOGameplayTags::Event_Plunge_Land, nullptr, true, true);
    WaitLandEvent->EventReceived.AddDynamic(this, &UKOGA_Attack_Plunge::OnLandEventReceived);
    WaitLandEvent->ReadyForActivation();

    CurrentMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this, NAME_None, LandMontage, 1.0f);
    CurrentMontageTask->OnCompleted.AddDynamic(this, &UKOGA_Attack_Plunge::OnMontageCompleted);
    CurrentMontageTask->OnCancelled.AddDynamic(this, &UKOGA_Attack_Plunge::OnMontageCancelled);
    CurrentMontageTask->OnInterrupted.AddDynamic(this, &UKOGA_Attack_Plunge::OnMontageCancelled);
    CurrentMontageTask->ReadyForActivation();
}

void UKOGA_Attack_Plunge::OnLandEventReceived(FGameplayEventData Payload)
{
    ApplyPlungeImpact(Payload);
}


void UKOGA_Attack_Plunge::OnMontageCancelled()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UKOGA_Attack_Plunge::OnMontageCompleted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
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

void UKOGA_Attack_Plunge::StopCurrentMontageTask()
{
    if (!CurrentMontageTask) { return; }

    CurrentMontageTask->OnCompleted.RemoveAll(this);
    CurrentMontageTask->OnCancelled.RemoveAll(this);
    CurrentMontageTask->OnInterrupted.RemoveAll(this);
    CurrentMontageTask->EndTask();
    CurrentMontageTask = nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
// 헬퍼: 착지 지점 기준 범위 내 적에게 차징 배율 피해 적용
// ─────────────────────────────────────────────────────────────────────────────
void UKOGA_Attack_Plunge::ApplyPlungeImpact(const FGameplayEventData& Payload)
{
    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    if (!AvatarActor) { return; }

    const float DamageMultiplier = FMath::Lerp(1.f, MaxDamageMultiplier, ChargeRatio);
    const FVector ImpactLocation = AvatarActor->GetActorLocation();

    TArray<FOverlapResult> OverlapResults;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(AvatarActor);

    GetWorld()->OverlapMultiByChannel(
        OverlapResults,
        ImpactLocation,
        FQuat::Identity,
        ECC_Pawn,
        FCollisionShape::MakeSphere(ImpactRadius),
        QueryParams
    );

    for (const FOverlapResult& Result : OverlapResults)
    {
        AActor* HitActor = Result.GetActor();
        if (!IsValid(HitActor) || !HitActor->ActorHasTag(FName("Enemy"))) { continue; }

        FGameplayEventData TargetPayload  = Payload;
        TargetPayload.Target              = HitActor;
        TargetPayload.Instigator          = AvatarActor;
        TargetPayload.EventMagnitude      = DamageMultiplier;

        SendAttackEventsToTarget(&TargetPayload);
        ApplyHitEffects(&TargetPayload);
    }
}
