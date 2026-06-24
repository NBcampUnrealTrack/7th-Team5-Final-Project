#include "AbilitySystem/Ability/Enemy/Boss/Attack/KOGA_BossJumpAttack.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Character/Enemy/Boss/KOBossBase.h"
#include "GameFramework/CharacterMovementComponent.h"

UKOGA_BossJumpAttack::UKOGA_BossJumpAttack()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UKOGA_BossJumpAttack::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (!CommitAbilityCost(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

AKOBossBase* Boss = Cast<AKOBossBase>(GetAvatarActorFromActorInfo());    if (!Boss)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    
    LaunchTargetLocation = Boss->JumpTargetLocation;
    
    UAbilityTask_PlayMontageAndWait* MontageTask =
        UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this,
            NAME_None,
            AttackMontage,
            1.f,
            JumpSection);

    MontageTask->OnCompleted.AddDynamic(
        this, &UKOGA_BossJumpAttack::OnMontageCompleted);
    MontageTask->OnCancelled.AddDynamic(
        this, &UKOGA_BossJumpAttack::OnMontageCancelled);
    MontageTask->ReadyForActivation();
}

void UKOGA_BossJumpAttack::Launch()
{
AKOBossBase* Boss = Cast<AKOBossBase>(GetAvatarActorFromActorInfo());    if (!Boss) return;

    const FVector BossLocation  = Boss->GetActorLocation();
    const FVector ToTarget      = LaunchTargetLocation - BossLocation;
    const FVector HorizontalDir = FVector(ToTarget.X, ToTarget.Y, 0.f).GetSafeNormal();
    const float   Distance      = FVector(ToTarget.X, ToTarget.Y, 0.f).Size();
    
    const float Vz      =  4.f * JumpHeight / JumpTime;
    const float Vxy     =  Distance / JumpTime;
    const float Gravity = -8.f * JumpHeight / (JumpTime * JumpTime);

    UCharacterMovementComponent* MoveComp = Boss->GetCharacterMovement();
    MoveComp->GravityScale = Gravity / GetWorld()->GetGravityZ();

    Boss->LaunchCharacter(
        HorizontalDir * Vxy + FVector::UpVector * Vz,
        true, true);

    // Float 섹션 루프 전환
    UAnimInstance* AnimInst = Boss->GetMesh()->GetAnimInstance();
    if (AnimInst)
    {
        AnimInst->Montage_JumpToSection(FloatSection, AttackMontage);
    }

    BindLandedDelegate();

    // 체공 유도 타이머
    FTimerDelegate HomingDelegate;
    HomingDelegate.BindUObject(this, &UKOGA_BossJumpAttack::UpdateHoming);
    GetWorld()->GetTimerManager().SetTimer(
        HomingTimerHandle,
        HomingDelegate,
        0.1f,
        true);
}

void UKOGA_BossJumpAttack::UpdateHoming()
{
    AKOBossBase* Boss = Cast<AKOBossBase>(GetAvatarActorFromActorInfo());    
    if (!Boss) return;

    UCharacterMovementComponent* MoveComp = Boss->GetCharacterMovement();
    const FVector CurrentVelocity = MoveComp->Velocity;
    const FVector CurrentHorizontal = FVector(CurrentVelocity.X, CurrentVelocity.Y, 0.f);
    const float   HorizontalSpeed = CurrentHorizontal.Size();

    if (HorizontalSpeed < SMALL_NUMBER) 
    {
        return;
    }

    const FVector ToTarget = LaunchTargetLocation - Boss->GetActorLocation();
    const FVector TargetDir = FVector(ToTarget.X, ToTarget.Y, 0.f).GetSafeNormal();
    const FVector CurrentDir = CurrentHorizontal.GetSafeNormal();
    const FVector NewDir = FMath::Lerp(CurrentDir, TargetDir, HomingStrength).GetSafeNormal();

    // 수평 방향만 보정, Z속도는 물리 그대로 유지
    const FVector NewVelocity = NewDir * HorizontalSpeed;
    MoveComp->Velocity = FVector(NewVelocity.X, NewVelocity.Y, CurrentVelocity.Z);
}

void UKOGA_BossJumpAttack::OnCharacterLanded(const FHitResult& Hit)
{
    GetWorld()->GetTimerManager().ClearTimer(HomingTimerHandle);
    UnbindLandedDelegate();

    AKOBossBase* Boss = Cast<AKOBossBase>(GetAvatarActorFromActorInfo());    
    if (!Boss)
    {
        return;
    }
    
    Boss->GetCharacterMovement()->GravityScale = 1.f;
    
    UAnimInstance* AnimInst = Boss->GetMesh()->GetAnimInstance();
    if (AnimInst)
    {
        AnimInst->Montage_JumpToSection(LandSection, AttackMontage);
    }
}

void UKOGA_BossJumpAttack::BindLandedDelegate()
{
    AKOBossBase* Boss = Cast<AKOBossBase>(GetAvatarActorFromActorInfo());    
    if (!Boss)
    {
        return;
    }
    
    Boss->LandedDelegate.AddDynamic(this, &UKOGA_BossJumpAttack::OnCharacterLanded);
}

void UKOGA_BossJumpAttack::UnbindLandedDelegate()
{
    AKOBossBase* Boss = Cast<AKOBossBase>(GetAvatarActorFromActorInfo());    
    if (!Boss)
    {
        return;
    }
    
    Boss->LandedDelegate.RemoveDynamic(this, &UKOGA_BossJumpAttack::OnCharacterLanded);
}

void UKOGA_BossJumpAttack::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
    // 비정상 종료 시 보험
    GetWorld()->GetTimerManager().ClearTimer(HomingTimerHandle);
    UnbindLandedDelegate();

    AKOBossBase* Boss = Cast<AKOBossBase>(GetAvatarActorFromActorInfo());
    if (Boss)
    {
        Boss->GetCharacterMovement()->GravityScale = 1.f;
    }

    LaunchTargetLocation = FVector::ZeroVector;

    ApplyCooldown(Handle, ActorInfo, ActivationInfo);
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
