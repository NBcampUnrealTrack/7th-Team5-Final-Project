#include "KOGA_Utility_LockOn.h"
#include "Character/Hero/KOHeroCharacter.h"
#include "Character/Enemy/Boss/KOBossBase.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Character/Enemy/KOBaseEnemy.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Game/KOPlayerController.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/OverlapResult.h"
#include "Camera/CameraComponent.h"

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
	
	SetCanBeCanceled(false);
	
	// 에너미 락온 UI 활성화
	if (AKOBaseEnemy* Enemy=Cast<AKOBaseEnemy>(LockedTarget))
	{
		Enemy->OnLockOnEvent.ExecuteIfBound(true);
	}
 
	if (UWorld* World = GetWorld())
	{
		LockOnActivationTime = World->GetTimeSeconds();
		if (bShowDebugMessages && GEngine)
		{
			GEngine->AddOnScreenDebugMessage(11, 2.f, FColor::Green, TEXT("[LockOn] Activated"));
		}
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
 
	if (UCameraComponent* Cam = OwnerChar->FindComponentByClass<UCameraComponent>())
	{
		DefaultCameraRelLocation = Cam->GetRelativeLocation(); // 해제 시 복구용
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
	
	// 에너미 락온 UI 비활성화
	if (AKOBaseEnemy* Enemy=Cast<AKOBaseEnemy>(LockedTarget))
	{
		Enemy->OnLockOnEvent.ExecuteIfBound(false);
	}
	
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
		
		if (UCameraComponent* Cam = OwnerChar->FindComponentByClass<UCameraComponent>())
		{
			Cam->SetRelativeLocation(DefaultCameraRelLocation);
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

    	// 죽은 적은 후보에서 제외 (시체에 다시 락온되는 것 방지)
    	if (IAbilitySystemInterface* DeadIface = Cast<IAbilitySystemInterface>(HitActor))
    	{
    		if (UAbilitySystemComponent* DeadASC = DeadIface->GetAbilitySystemComponent())
    		{
    			if (DeadASC->HasMatchingGameplayTag(KOGameplayTags::State_Character_Dead) ||
					DeadASC->HasMatchingGameplayTag(KOGameplayTags::State_Boss_Dead))
    			{
    				continue;
    			}
    		}
    	}
        // 1. 일반 액터 태그 검사
    	bool bIsEnemy = HitActor->ActorHasTag(EnemyActorTag) || HitActor->IsA(AKOBaseEnemy::StaticClass());
    	bool bIsBoss  = HitActor->ActorHasTag(BossActorTag)  || HitActor->IsA(AKOBossBase::StaticClass());
   
        if (!bIsEnemy && !bIsBoss) continue;

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
	if (!LockedTarget.IsValid()) return false;

	AActor* TargetActor = LockedTarget.Get();
	ACharacter* OwnerChar = GetAvatarCharacter();

	// 2. GAS 사망 태그 여부 체크
	if (IAbilitySystemInterface* ASCIface = Cast<IAbilitySystemInterface>(TargetActor))
	{
		UAbilitySystemComponent* TargetASC = ASCIface->GetAbilitySystemComponent();
		if (TargetASC)
		{
			if (TargetASC->HasMatchingGameplayTag(KOGameplayTags::State_Character_Dead) ||
				TargetASC->HasMatchingGameplayTag(KOGameplayTags::State_Boss_Dead))
			{
				if (bShowDebugMessages) GEngine->AddOnScreenDebugMessage(30, 1.f, FColor::Red, TEXT("[Valid] Dead 태그 감지됨"));
				return false;
			}
		}
	}

	
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

	AKOPlayerController* PC = Cast<AKOPlayerController>(OwnerChar->GetController());
	if (!PC) return;

	const float DeltaTime = GetWorld()->GetDeltaSeconds();
	
	const FVector EyeLoc = OwnerChar->GetActorLocation();
	//FRotator AnchorRot = UKismetMathLibrary::FindLookAtRotation(EyeLoc, GetTargetSocketLocation());
	FRotator AnchorRot = UKismetMathLibrary::FindLookAtRotation(EyeLoc, LockedTarget->GetActorLocation());
	const bool bTargetIsBoss =
		LockedTarget.IsValid() && LockedTarget->IsA(AKOBossBase::StaticClass());
	
	AnchorRot.Pitch = bTargetIsBoss ? BossLockOnCameraPitch : LockOnCameraPitch;
	
	
	if (UCameraComponent* Cam = OwnerChar->FindComponentByClass<UCameraComponent>())
	{
		// 일반 몬스터: 카메라 기본 위치 유지(= 현재 동작과 동일).
		FVector TargetRelLoc = DefaultCameraRelLocation;

		// 보스만: 오프셋 + 뒤로 빼기(줌아웃) 적용
		if (bTargetIsBoss)
		{
			TargetRelLoc = LockOnCameraOffset;

			// 타겟이 가까울수록 카메라를 더 뒤로 뺀다 (0 ~ MaxCloseExtraArmLength)
			const float DistToTarget = FVector::Dist(EyeLoc, LockedTarget->GetActorLocation());
			float CloseExtra = 0.f;
			if (DistToTarget < CloseDistanceThreshold)
			{
				const float Alpha = 1.f - (DistToTarget / CloseDistanceThreshold);
				CloseExtra = Alpha * MaxCloseExtraArmLength;
			}

			// 카메라를 뒤로(-X) 빼서 보스 덩치가 화면에 들어오게 함
			TargetRelLoc.X -= (BossLockOnExtraArmLength + CloseExtra);
		}

		const FVector NewRelLoc = FMath::VInterpTo(
			Cam->GetRelativeLocation(), TargetRelLoc, DeltaTime, CameraOffsetInterpSpeed);
		Cam->SetRelativeLocation(NewRelLoc);
	}

	
	// 현재 시점(이번 프레임 마우스 입력이 이미 반영된 상태)
	const FRotator CurrentRot = PC->GetControlRotation();

	// 최근 마우스 조작 여부
	const bool bRecentering = PC->GetTimeSinceLastLookInput() > ReactivateDelay;

	FRotator TargetRot;
	if (bRecentering)
	{
		// 손을 뗐으면 → 앵커(정면)로 복귀
		TargetRot = AnchorRot;
	}
	else
	{
		// 조작 중이면 → 리시 범위 안으로만 클램프
		const float YawDelta   = FMath::Clamp(FRotator::NormalizeAxis(CurrentRot.Yaw   - AnchorRot.Yaw),   -LeashYaw,   LeashYaw);
		const float PitchDelta = FMath::Clamp(FRotator::NormalizeAxis(CurrentRot.Pitch - AnchorRot.Pitch), -LeashPitch, LeashPitch);
		TargetRot = AnchorRot + FRotator(PitchDelta, YawDelta, 0.f);
	}

	const float Speed = bRecentering ? RecenterInterpSpeed : CameraInterpSpeed;
	const FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, Speed);
	PC->SetControlRotation(NewRot);

	// 몸통은 계속 타겟을 향하도록 수동 회전 (bUseControllerRotationYaw=false 이므로)
	FRotator BodyRot = OwnerChar->GetActorRotation();
	BodyRot.Yaw = FMath::RInterpTo(BodyRot, FRotator(0.f, AnchorRot.Yaw, 0.f), DeltaTime, CameraInterpSpeed).Yaw;
	
}
 
void UKOGA_Utility_LockOn::StartCameraUpdate()
{
	StopCameraUpdate(); // 중복 등록 방지

	CameraTickHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &UKOGA_Utility_LockOn::CameraTick));
}

bool UKOGA_Utility_LockOn::CameraTick(float /*DeltaTime*/)
{
	// UpdateCameraRotation 내부에서 GetWorld()->GetDeltaSeconds()로
	// 실제 프레임 델타를 사용하므로 여기선 그대로 호출만 한다.
	UpdateCameraRotation();
	return true; // true를 반환해야 다음 프레임에도 계속 틱
}



void UKOGA_Utility_LockOn::StopCameraUpdate()
{
	if (CameraTickHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(CameraTickHandle);
		CameraTickHandle.Reset();
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


