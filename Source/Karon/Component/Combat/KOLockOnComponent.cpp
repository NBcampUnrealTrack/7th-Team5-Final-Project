// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Combat/KOLockOnComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"  
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"



// Sets default values for this component's properties
UKOLockOnComponent::UKOLockOnComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
}


// Called when the game starts
void UKOLockOnComponent::BeginPlay()
{
	Super::BeginPlay();
	
	
}


// Called every frame
void UKOLockOnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
    if (!bIsLockedOn) return;

    if (!IsTargetValid())
    {
        DeactivateLockOn();
        return;
    }

    UpdateCameraRotation(DeltaTime);
}

void UKOLockOnComponent::ActivateLockOn()
{
    AActor* BestTarget = FindBestTarget();
    if (!BestTarget) return;

    LockedTarget = BestTarget;
    bIsLockedOn = true;

    if (ACharacter* OwnerChar = Cast<ACharacter>(GetOwner()))
    {
        OwnerChar->bUseControllerRotationYaw = true;
        // 락온 중: 이동 방향 자동 회전 OFF (카메라 방향 고정)
        OwnerChar->GetCharacterMovement()->bOrientRotationToMovement = false;
    }

    ApplyLockOnGameplayTag(true);
    OnLockOnChanged.Broadcast(true);
    OnTargetChanged.Broadcast(LockedTarget);
}

void UKOLockOnComponent::DeactivateLockOn()
{
    bIsLockedOn = false;
    LockedTarget = nullptr;

    if (ACharacter* OwnerChar = Cast<ACharacter>(GetOwner()))
    {
        OwnerChar->bUseControllerRotationYaw = false;
        // 락온 해제: 이동 방향 자동 회전 ON
        OwnerChar->GetCharacterMovement()->bOrientRotationToMovement = true;
    }

    ApplyLockOnGameplayTag(false);
    OnLockOnChanged.Broadcast(false);
    OnTargetChanged.Broadcast(nullptr);
}

void UKOLockOnComponent::SwitchTarget(bool bSwitchRight)
{
    if (!bIsLockedOn || !LockedTarget) return;

    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    if (!OwnerChar) return;

    TArray<AActor*> Enemies;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Enemy"), Enemies);

    AActor* BestCandidate = nullptr;
    float BestAngle = 180.f;

    FVector OwnerLoc = OwnerChar->GetActorLocation();
    FVector ToCurrentTarget = (LockedTarget->GetActorLocation() - OwnerLoc).GetSafeNormal();

    for (AActor* Enemy : Enemies)
    {
        if (!Enemy || Enemy == LockedTarget) continue;

        float Distance = FVector::Dist(OwnerLoc, Enemy->GetActorLocation());
        if (Distance > SearchRadius) continue;

        FVector ToEnemy = (Enemy->GetActorLocation() - OwnerLoc).GetSafeNormal();
        FVector Cross = FVector::CrossProduct(ToCurrentTarget, ToEnemy);
        bool bIsRight = Cross.Z < 0;

        if (bSwitchRight != bIsRight) continue;

        float Angle = FMath::Acos(FVector::DotProduct(ToCurrentTarget, ToEnemy));
        if (Angle < BestAngle)
        {
            BestAngle = Angle;
            BestCandidate = Enemy;
        }
    }

    if (BestCandidate)
    {
        LockedTarget = BestCandidate;
        OnTargetChanged.Broadcast(LockedTarget);
    }
}

FVector UKOLockOnComponent::GetTargetSocketLocation() const
{
    if (!LockedTarget) return FVector::ZeroVector;

    if (ACharacter* TargetChar = Cast<ACharacter>(LockedTarget))
    {
        USkeletalMeshComponent* Mesh = TargetChar->GetMesh();
        if (Mesh && Mesh->DoesSocketExist(TargetSocketName))
        {
            return Mesh->GetSocketLocation(TargetSocketName);
        }
    }

    return LockedTarget->GetActorLocation();
}

void UKOLockOnComponent::UpdateCameraRotation(float DeltaTime) const
{
    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    if (!OwnerChar) return;

    APlayerController* PC = Cast<APlayerController>(OwnerChar->GetController());
    if (!PC) return;

    FVector CameraLoc;
    FRotator CameraRot;
    PC->GetPlayerViewPoint(CameraLoc, CameraRot);

    FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(
        CameraLoc,
        GetTargetSocketLocation()
    );

    FRotator NewRot = FMath::RInterpTo(
        PC->GetControlRotation(),
        TargetRot,
        DeltaTime,
        CameraInterpSpeed
    );

    PC->SetControlRotation(NewRot);
}

void UKOLockOnComponent::ApplyLockOnGameplayTag(bool bApply) const
{
    IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(GetOwner());
    if (!ASCInterface) return;

    UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent();
    if (!ASC) return;

    // 에디터 설정 대신 네이티브 태그 직접 사용
    if (bApply)
        ASC->AddLooseGameplayTag(KOGameplayTags::State_Character_LockOn);
    else
        ASC->RemoveLooseGameplayTag(KOGameplayTags::State_Character_LockOn);
}

AActor* UKOLockOnComponent::FindBestTarget() const
{
    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    if (!OwnerChar) return nullptr;

    APlayerController* PC = Cast<APlayerController>(OwnerChar->GetController());
    if (!PC) return nullptr;

    FVector CameraLoc;
    FRotator CameraRot;
    PC->GetPlayerViewPoint(CameraLoc, CameraRot);
    FVector CameraForward = CameraRot.Vector();

    TArray<AActor*> Enemies;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Enemy"), Enemies);

    AActor* BestTarget = nullptr;
    float BestScore = -1.f;

    for (AActor* Enemy : Enemies)
    {
        if (!Enemy) continue;

        float Distance = FVector::Dist(OwnerChar->GetActorLocation(), Enemy->GetActorLocation());
        if (Distance > SearchRadius) continue;

        FVector ToEnemy = (Enemy->GetActorLocation() - CameraLoc).GetSafeNormal();
        float DotScore = FVector::DotProduct(CameraForward, ToEnemy);

        if (DotScore > BestScore)
        {
            BestScore = DotScore;
            BestTarget = Enemy;
        }
    }

    return BestTarget;
}

bool UKOLockOnComponent::IsTargetValid() const
{
    if (!IsValid(LockedTarget)) return false;

    float Distance = FVector::Dist(
        GetOwner()->GetActorLocation(),
        LockedTarget->GetActorLocation()
    );

    return Distance <= SearchRadius * 1.2f;
}