#include "AbilitySystem/Ability/Enemy/Boss/Attack/KOGA_BossJumpAttack.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
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
    UGameplayAbility::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
 
    if (!CommitAbilityCost(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
 
    AKOBossBase* Boss = Cast<AKOBossBase>(GetAvatarActorFromActorInfo());
    if (!Boss)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
 
    LaunchTargetLocation = Boss->JumpTargetLocation;
    
    WaitLaunchEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
        this, 
        LaunchEventTag, 
        nullptr, 
        false, 
        false
        );
        
    WaitLaunchEventTask->EventReceived.AddDynamic(this, &UKOGA_BossJumpAttack::OnLaunchEventReceived);
    WaitLaunchEventTask->ReadyForActivation();
 
    UAbilityTask_PlayMontageAndWait* MontageTask =
        UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this,
            NAME_None,
            AttackMontage,
            1.f,
            JumpSection);
    
    MontageTask->OnCompleted.AddDynamic(this, &UKOGA_BossJumpAttack::OnMontageCompleted);
    MontageTask->OnCancelled.AddDynamic(this, &UKOGA_BossJumpAttack::OnMontageCancelled);
    MontageTask->OnInterrupted.AddDynamic(this, &UKOGA_BossJumpAttack::OnMontageCancelled);
    MontageTask->ReadyForActivation();
}

void UKOGA_BossJumpAttack::OnLaunchEventReceived(FGameplayEventData Payload)
{
    Launch();
}
 
void UKOGA_BossJumpAttack::Launch()
{
    AKOBossBase* Boss = Cast<AKOBossBase>(GetAvatarActorFromActorInfo());
    if (!Boss)
    {
        return;
    }
    const FVector BossLocation  = Boss->GetActorLocation();
    const FVector ToTarget      = LaunchTargetLocation - BossLocation;
    const FVector HorizontalDir = FVector(ToTarget.X, ToTarget.Y, 0.f).GetSafeNormal();
    const float   Distance      = FVector(ToTarget.X, ToTarget.Y, 0.f).Size();
    
    const float Vz      =  4.f * JumpHeight / JumpTime;
    const float Vxy     =  Distance / JumpTime;
    const float Gravity = -8.f * JumpHeight / (JumpTime * JumpTime);
 
    UCharacterMovementComponent* MoveComp = Boss->GetCharacterMovement();
    
    BaseGravity = Gravity;
    CurrentGravityPhase = EJumpGravityPhase::Rising;
    MoveComp->GravityScale = (BaseGravity * RiseGravityMultiplier) / GetWorld()->GetGravityZ();
    
    BaseHorizontalSpeed = Vxy;

    GetWorld()->GetTimerManager().ClearTimer(FloatHoldTimerHandle);
 
    Boss->LaunchCharacter(
        HorizontalDir * (Vxy * RiseHorizontalRatio) + FVector::UpVector * Vz,
        true, true);
    
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
    if (!Boss)
    {
        return;
    }
    UCharacterMovementComponent* MoveComp = Boss->GetCharacterMovement();
    const FVector CurrentVelocity = MoveComp->Velocity;
    const FVector CurrentHorizontal = FVector(CurrentVelocity.X, CurrentVelocity.Y, 0.f);
    const float   HorizontalSpeed = CurrentHorizontal.Size();
    
    if (CurrentGravityPhase == EJumpGravityPhase::Rising && CurrentVelocity.Z <= 0.f)
    {
        CurrentGravityPhase = EJumpGravityPhase::Floating;
        MoveComp->GravityScale = (BaseGravity * FloatGravityMultiplier) / GetWorld()->GetGravityZ();

        GetWorld()->GetTimerManager().SetTimer(
            FloatHoldTimerHandle,
            this,
            &UKOGA_BossJumpAttack::StartFalling,
            FloatHoldTime,
            false);
    }
    const float DesiredSpeed = BaseHorizontalSpeed;

    const FVector LiveTargetLoc = Boss->CurrentTarget ?
        Boss->CurrentTarget->GetActorLocation() : LaunchTargetLocation;
    const FVector ToTarget = LiveTargetLoc - Boss->GetActorLocation();
    const FVector TargetDir = FVector(ToTarget.X, ToTarget.Y, 0.f).GetSafeNormal();
    
    const FVector CurrentDir = HorizontalSpeed > SMALL_NUMBER ? CurrentHorizontal.GetSafeNormal() : TargetDir;

    const FVector NewDir   = FMath::Lerp(CurrentDir, TargetDir, HomingStrength).GetSafeNormal();
    const float   NewSpeed = FMath::Lerp(HorizontalSpeed, DesiredSpeed, HomingStrength);

    // 수평 방향 속도만 보정, Z속도는 물리 그대로 유지
    const FVector NewVelocity = NewDir * NewSpeed;
    MoveComp->Velocity = FVector(NewVelocity.X, NewVelocity.Y, CurrentVelocity.Z);
}

void UKOGA_BossJumpAttack::StartFalling()
{
    AKOBossBase* Boss = Cast<AKOBossBase>(GetAvatarActorFromActorInfo());
    if (!Boss)
    {
        return;
    }
    
    CurrentGravityPhase = EJumpGravityPhase::Falling;

    UCharacterMovementComponent* MoveComp = Boss->GetCharacterMovement();
    MoveComp->GravityScale = (BaseGravity * FallGravityMultiplier) / GetWorld()->GetGravityZ();
}
 
void UKOGA_BossJumpAttack::OnCharacterLanded(const FHitResult& Hit)
{
    GetWorld()->GetTimerManager().ClearTimer(HomingTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(FloatHoldTimerHandle);
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

void UKOGA_BossJumpAttack::OnMontageCompleted()
{
    ApplyCooldown(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo);
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UKOGA_BossJumpAttack::OnMontageCancelled()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
 
void UKOGA_BossJumpAttack::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
    GetWorld()->GetTimerManager().ClearTimer(HomingTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(FloatHoldTimerHandle);
    UnbindLandedDelegate();
    
    if (WaitLaunchEventTask)
    {
        WaitLaunchEventTask->EndTask();
        WaitLaunchEventTask = nullptr;
    }
 
    AKOBossBase* Boss = Cast<AKOBossBase>(GetAvatarActorFromActorInfo());
    if (Boss)
    {
        Boss->GetCharacterMovement()->GravityScale = 1.f;
    }
 
    LaunchTargetLocation = FVector::ZeroVector;

    CurrentGravityPhase = EJumpGravityPhase::Rising;
    BaseGravity = 0.f;
    BaseHorizontalSpeed = 0.f;
 
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
