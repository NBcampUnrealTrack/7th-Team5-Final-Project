#include "KOGA_Utility_LockOn.h"
#include "Character/Hero/KOHeroCharacter.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/OverlapResult.h"


UKOGA_Utility_LockOn::UKOGA_Utility_LockOn()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	SetAssetTags(FGameplayTagContainer(KOGameplayTags::Input_Ability_Utility_LockOn));
}

// ─────────────────────────────────────────────────────────────────────
// ActivateAbility  ―  버튼 첫 누름 시 호출
// ───────────────────────────────────────────────────────────────────── 
void UKOGA_Utility_LockOn::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AKOHeroCharacter* HeroChar = Cast<AKOHeroCharacter>(GetAvatarCharacter());
	if (!HeroChar)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	// 1. 먼저 가장 적합한 타겟을 탐색
	AActor* BestTarget = FindBestTarget();
	if (!BestTarget)
	{
		if (bShowDebugMessages) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("[LockOn] 타겟이 없어 활성화 실패"));
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	// 2. 타겟을 찾은 경우에만 코스트 및 쿨타임 커밋 (12번 해결)
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	// 3. 락온 상태 설정 및 활성화
	LockedTarget = BestTarget;
	bIsLockedOn = true;
	
 
	if (UWorld* World = GetWorld())
	{
		LockOnActivationTime = World->GetTimeSeconds();
		GEngine->AddOnScreenDebugMessage(11, 2.f, FColor::Green, TEXT("[LockOn] Activated"));
	}
	ActivateLockOn();
}

// ─────────────────────────────────────────────────────────────────────
// InputPressed  ―  어빌리티 Active 상태에서 버튼 재누름 시 호출
// ─────────────────────────────────────────────────────────────────────
void UKOGA_Utility_LockOn::InputPressed(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (UWorld* World = GetWorld())
	{
		float CurrentTime = World->GetTimeSeconds();
		if (CurrentTime - LockOnActivationTime < 0.15f)
		{
			return; 
		}
	}
	
	DeactivateLockOn(); //
 
	if (bShowDebugMessages)
	{
		GEngine->AddOnScreenDebugMessage(11, 2.f, FColor::Red, TEXT("[LockOn] Deactivated (Button)")); //
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

// ─────────────────────────────────────────────────────────────────────
// EndAbility  ―  어빌리티 종료 시 항상 호출 (클린업)
// ─────────────────────────────────────────────────────────────────────
void UKOGA_Utility_LockOn::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
	// 모든 타이머 정지 및 상태 복구
	StopCameraUpdate();
	StopLockOnDistanceCheck();
	DeactivateLockOn();
 
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

// ─────────────────────────────────────────────────────────────────────
// ActivateLockOn  ―  락온 켜기
// ─────────────────────────────────────────────────────────────────────
void UKOGA_Utility_LockOn::ActivateLockOn()
{
	ACharacter* OwnerChar = GetAvatarCharacter();
	if (!OwnerChar) return;
 
	// 락온 회전 제어 설정
	OwnerChar->bUseControllerRotationYaw = true;
	if (UCharacterMovementComponent* MoveComp = OwnerChar->GetCharacterMovement())
	{
		MoveComp->bOrientRotationToMovement = false;
	}
 
	ApplyLockOnGameplayTag(true);
 
	StartCameraUpdate();
	StartLockOnDistanceCheck();
}
 
 
// ─────────────────────────────────────────────────────────────────────
// DeactivateLockOn  ―  락온 끄기
// ─────────────────────────────────────────────────────────────────────
void UKOGA_Utility_LockOn::DeactivateLockOn()
{
	if (!bIsLockedOn) return;
	bIsLockedOn = false;
	LockedTarget = nullptr;
 
	ACharacter* OwnerChar = GetAvatarCharacter();
	if (OwnerChar)
	{
		OwnerChar->bUseControllerRotationYaw = false;
		if (UCharacterMovementComponent* MoveComp = OwnerChar->GetCharacterMovement())
		{
			MoveComp->bOrientRotationToMovement = true;
		}
	}
	
	ApplyLockOnGameplayTag(false);
}
 
 
// ─────────────────────────────────────────────────────────────────────
// FindBestTarget  ―  카메라 정면에서 가장 가까운 적 탐색
// ─────────────────────────────────────────────────────────────────────
AActor* UKOGA_Utility_LockOn::FindBestTarget() const
{
    ACharacter* OwnerChar = GetAvatarCharacter();
    if (!OwnerChar) return nullptr;

    APlayerController* PC = Cast<APlayerController>(OwnerChar->GetController());
    /*FVector CameraForward = PC
        ? PC->GetControlRotation().Vector()
        : OwnerChar->GetActorForwardVector();*/

    TArray<FOverlapResult> OverlapResults;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(SearchRadius);
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerChar);

    FCollisionObjectQueryParams ObjParams;
    ObjParams.AddObjectTypesToQuery(ECC_Pawn);

    GetWorld()->OverlapMultiByObjectType(
        OverlapResults,
        OwnerChar->GetActorLocation(),
        FQuat::Identity,
        ObjParams,
        Sphere,
        Params
    );

    // ── 진단 로그 1: Overlap 결과 총 개수
    GEngine->AddOnScreenDebugMessage(20, 3.f, FColor::White,
        FString::Printf(TEXT("[LockOn] Overlap 결과: %d개 (반경 %.0f)"),
                        OverlapResults.Num(), SearchRadius));

    AActor* BestTarget = nullptr;
    float BestScore = -FLT_MAX;

    for (auto& Result : OverlapResults)
    {
        AActor* HitActor = Result.GetActor();
        if (!IsValid(HitActor)) continue;

        // 1. 일반 액터 태그 검사
        bool bIsEnemy = HitActor->ActorHasTag(EnemyActorTag);
        bool bIsBoss  = HitActor->ActorHasTag(FName("Boss"));

        // 2. GAS 게임플레이 태그 검사 보완 (액터 태그가 없을 때를 대비한 안전망)
        if (!bIsEnemy && !bIsBoss)
        {
            if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(HitActor))
            {
                if (UAbilitySystemComponent* TargetASC = ASCInterface->GetAbilitySystemComponent())
                {
                    bIsEnemy = TargetASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Enemy")));
                    bIsBoss  = TargetASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Boss")));
                }
            }
        }
    	
   
        if (!bIsEnemy && !bIsBoss) continue;

        //FVector ToTarget = (HitActor->GetActorLocation() - OwnerChar->GetActorLocation()).GetSafeNormal();

        //float Dot      = FVector::DotProduct(CameraForward, ToTarget);
        float NormDist = FVector::Dist(OwnerChar->GetActorLocation(), HitActor->GetActorLocation()) / SearchRadius;
        float Score    = 1.f - NormDist;

        if (Score > BestScore)
        {
            BestScore  = Score;
            BestTarget = HitActor;
        }
    }

    // ── 진단 로그 3: 최종 선택된 타겟
    GEngine->AddOnScreenDebugMessage(21, 3.f, BestTarget ? FColor::Green : FColor::Red,
        FString::Printf(TEXT("[LockOn] 선택된 타겟: %s"),
            BestTarget ? *BestTarget->GetName() : TEXT("없음")));
	
    return BestTarget;
}
 
// ─────────────────────────────────────────────────────────────────────
// IsTargetValid  ―  타겟이 아직 유효한지 확인
// ─────────────────────────────────────────────────────────────────────
bool UKOGA_Utility_LockOn::IsTargetValid() const
{
	// 1. 약참조 유효성 체크
	/*if (!LockedTarget.IsValid())
	{
		if (bShowDebugMessages) GEngine->AddOnScreenDebugMessage(30, 1.f, FColor::Red, TEXT("[Valid] Target 소멸됨 (null)"));
		return false;
	}

	AActor* TargetActor = LockedTarget.Get();*/
	if (!LockedTarget.IsValid()) return false;

	AActor* TargetActor = LockedTarget.Get();
	ACharacter* OwnerChar = GetAvatarCharacter();

	// 2. GAS 사망 태그 여부 체크
	if (IAbilitySystemInterface* ASCIface = Cast<IAbilitySystemInterface>(TargetActor))
	{
		UAbilitySystemComponent* TargetASC = ASCIface->GetAbilitySystemComponent();
		if (TargetASC)
		{
			if (TargetASC->HasMatchingGameplayTag(KOGameplayTags::State_Enemy_Dead) ||
				TargetASC->HasMatchingGameplayTag(KOGameplayTags::State_Boss_Dead))
			{
				if (bShowDebugMessages) GEngine->AddOnScreenDebugMessage(30, 1.f, FColor::Red, TEXT("[Valid] Dead 태그 감지됨"));
				return false;
			}
		}
	}

	// LOS 체크 — 타겟 소켓 위치로 LineTrace
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerChar);
	Params.AddIgnoredActor(TargetActor);

	FVector Start = OwnerChar->GetActorLocation();
	FVector End   = GetTargetSocketLocation();

	bool bBlocked = GetWorld()->LineTraceSingleByChannel(
		HitResult, Start, End, ECC_Visibility, Params);

	if (bBlocked) return false;
	
	// 3. 거리 체크
	float Distance = FVector::Dist(GetAvatarCharacter()->GetActorLocation(), TargetActor->GetActorLocation());
	return Distance <= LockOnBreakDistance;
}
 
 
// ─────────────────────────────────────────────────────────────────────
// GetTargetSocketLocation  ―  타겟의 LockOnSocket 위치 반환
// ─────────────────────────────────────────────────────────────────────
FVector UKOGA_Utility_LockOn::GetTargetSocketLocation() const
{
	if (!LockedTarget.IsValid()) return FVector::ZeroVector;
 
	if (ACharacter* TargetChar = Cast<ACharacter>(LockedTarget.Get()))
	{
		USkeletalMeshComponent* Mesh = TargetChar->GetMesh();
		if (Mesh && Mesh->DoesSocketExist(TargetSocketName))
		{
			return Mesh->GetSocketLocation(TargetSocketName);
		}
	}
 
	return LockedTarget->GetActorLocation();
}
 
 
// ─────────────────────────────────────────────────────────────────────
// UpdateCameraRotation  ―  카메라를 타겟 쪽으로 보간 회전 (타이머 콜백)
// ─────────────────────────────────────────────────────────────────────
void UKOGA_Utility_LockOn::UpdateCameraRotation()
{
	if (!IsTargetValid())
	{
		// 타겟이 유효하지 않으면 즉시 타겟 재탐색 시도
		AActor* NewTarget = FindBestTarget();
		if (NewTarget)
		{
			LockedTarget = NewTarget;
		}
		else
		{
			// 정말 주변에 타겟이 없을 때 안전하게 예약 종료 (1번 해결)
			StopCameraUpdate();
			StopLockOnDistanceCheck();
			
			GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
			{
				EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
			});
			return;
		}
	}

	ACharacter* OwnerChar = GetAvatarCharacter();
	if (!OwnerChar) return;

	APlayerController* PC = Cast<APlayerController>(OwnerChar->GetController());
	if (!PC) return;

	FVector  CameraLoc;
	FRotator CameraRot;
	PC->GetPlayerViewPoint(CameraLoc, CameraRot);

	FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(CameraLoc, GetTargetSocketLocation());
	
	// 보스 타겟이면 더 높은 시야각(덜 내려다보는 각도) 사용
	bool bTargetIsBoss = false;
	if (LockedTarget.IsValid())
	{
		AActor* Target = LockedTarget.Get();
		bTargetIsBoss = Target->ActorHasTag(BossActorTag);

		if (!bTargetIsBoss)
		{
			if (IAbilitySystemInterface* ASCIface = Cast<IAbilitySystemInterface>(Target))
			{
				if (UAbilitySystemComponent* TargetASC = ASCIface->GetAbilitySystemComponent())
				{
					bTargetIsBoss = TargetASC->HasMatchingGameplayTag(
						FGameplayTag::RequestGameplayTag(FName("State.Boss")));
				}
			}
		}
	}
	
	TargetRot.Pitch = bTargetIsBoss ? BossLockOnCameraPitch : LockOnCameraPitch;
	//TargetRot.Pitch = LockOnCameraPitch; // 하드코딩 제거된 변수 사용

	// GetWorld()->GetDeltaSeconds()를 사용하여 프레임 독립적인 부드러운 보간 수행
	FRotator NewRot = FMath::RInterpTo(
		PC->GetControlRotation(),
		TargetRot,
		GetWorld()->GetDeltaSeconds(),
		CameraInterpSpeed
	);

	PC->SetControlRotation(NewRot);
	
	
}
 
void UKOGA_Utility_LockOn::StartCameraUpdate()
{
    UWorld* World = GetWorld();
    if (!World) return;
 
    // ≈ 60fps 간격으로 카메라 회전 갱신
    World->GetTimerManager().SetTimer(
        CameraUpdateTimerHandle,
        this,
        &UKOGA_Utility_LockOn::UpdateCameraRotation,
        0.016f,
        true   // 반복
    );
}
 
void UKOGA_Utility_LockOn::StopCameraUpdate()
{
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(CameraUpdateTimerHandle);
    }
}

// ─────────────────────────────────────────────────────────────────────
// CheckLockOnDistance  ―  0.2초마다 거리 확인 (타이머 콜백)
// ─────────────────────────────────────────────────────────────────────
void UKOGA_Utility_LockOn::CheckLockOnDistance()
{
	if (!LockedTarget.IsValid()) return;
 
	float Distance = FVector::Dist(GetAvatarCharacter()->GetActorLocation(), LockedTarget->GetActorLocation());
 
	if (bShowDebugMessages)
	{
		GEngine->AddOnScreenDebugMessage(1, 0.3f, FColor::Yellow,
			FString::Printf(TEXT("[LockOn] Distance: %.0f / Break: %.0f"), Distance, LockOnBreakDistance));
	}
 
	if (Distance >= LockOnBreakDistance)
	{
		StopCameraUpdate();
		StopLockOnDistanceCheck();
        
		if (bShowDebugMessages) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange, TEXT("[LockOn] 거리 초과로 해제"));

		// 안전하게 다음 틱에 어빌리티 종료 예약 (1번 해결)
		GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
		});
	}
}
 
void UKOGA_Utility_LockOn::StartLockOnDistanceCheck()
{
	UWorld* World = GetWorld();
	if (!World) return;
 
	World->GetTimerManager().SetTimer(
		LockOnDistanceTimerHandle,
		this,
		&UKOGA_Utility_LockOn::CheckLockOnDistance,
		0.2f,
		true
	);
}
 
void UKOGA_Utility_LockOn::StopLockOnDistanceCheck()
{
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(LockOnDistanceTimerHandle);
    }
}

// ─────────────────────────────────────────────────────────────────────
// ApplyLockOnGameplayTag  ―  State.Character.LockOn 태그 추가/제거
// ─────────────────────────────────────────────────────────────────────
void UKOGA_Utility_LockOn::ApplyLockOnGameplayTag(bool bApply)
{
	IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(GetAvatarCharacter());
	if (!ASCInterface) return;
 
	UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent();
	if (!ASC) return;
 
	if (bApply)
		ASC->AddLooseGameplayTag(KOGameplayTags::State_Character_LockOn);
	else
		ASC->RemoveLooseGameplayTag(KOGameplayTags::State_Character_LockOn);
}


