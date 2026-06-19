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
	// 어빌리티 인스턴스를 액터당 1개 유지
	// → bIsLockedOn, LockedTarget 등 멤버 변수를 안전하게 보관 가능
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
 
	// 이 어빌리티를 발동시킬 입력 태그 등록
	//AbilityTags.AddTag(KOGameplayTags::Input_Ability_LockOn);
	SetAssetTags(FGameplayTagContainer(KOGameplayTags::Input_Ability_Utility_LockOn));
	// ★ ActivationOwnedTags 는 여기서 추가하지 않음 ★
	// ApplyLockOnGameplayTag()에서 직접 AddLooseGameplayTag / RemoveLooseGameplayTag 로 관리
	// → 두 곳에서 동시에 태그를 추가하면 참조 카운트가 꼬이는 문제 방지
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

	// Held 루프가 InputPressed() 직후 재활성화 시도하는 것을 차단
	if (bDeactivatedByInput)
	{
		bDeactivatedByInput = false;
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);  // bWasCancelled=true
		return;
	}
	
	// 캐릭터 유효성 확인
	AKOHeroCharacter* Character = Cast<AKOHeroCharacter>(GetAvatarCharacter());
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	// 락온 활성화 시도
	ActivateLockOn();
	
	// 타겟을 찾지 못했으면 어빌리티 바로 종료
	if (!bIsLockedOn)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}
 
	// ★ EndAbility를 호출하지 않고 여기서 반환 ★
	// → 어빌리티가 Active 상태로 유지되어 InputPressed()가 동작함
	GEngine->AddOnScreenDebugMessage(11, 2.f, FColor::Green, TEXT("[LockOn] Activated"));

	//TODO: 락온이 유지되는 동안 어빌리티 유지 
	// 현재는 끄거나 킬때만 잠시 어빌리티가 활성화되는 방식 . 
	// 컴포넌트에서 처리하는걸 여기로 옮기면 굳이 컴포넌트까지도 필요 없어질 가능성 있음. 
	
}

// ─────────────────────────────────────────────────────────────────────
// InputPressed  ―  어빌리티 Active 상태에서 버튼 재누름 시 호출
// ─────────────────────────────────────────────────────────────────────
void UKOGA_Utility_LockOn::InputPressed(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo)
{
	bDeactivatedByInput = true;  // ← Held 루프 차단 플래그 세팅
	// 락온 상태 해제
	DeactivateLockOn();
 
	// 어빌리티 종료 → EndAbility에서 타이머도 함께 정리됨
	//EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
 
	GEngine->AddOnScreenDebugMessage(11, 2.f, FColor::Red, TEXT("[LockOn] Deactivated (Button)"));

    EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
	//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("LockOn Deactivated"));
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
	// 타이머 두 개 모두 정지
	StopCameraUpdate();
	StopLockOnDistanceCheck();
 
	// 혹시 락온이 아직 켜져 있으면 여기서 반드시 끔
	// (외부에서 EndAbility가 강제 호출되는 경우 대비)
	if (bIsLockedOn)
	{
		DeactivateLockOn();
	}
	
	// TODO: 임시로 쿨타임 
	if (!bWasCancelled)
	ApplyCooldown(Handle, ActorInfo, ActivationInfo);
 
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

// ─────────────────────────────────────────────────────────────────────
// ActivateLockOn  ―  락온 켜기
// ─────────────────────────────────────────────────────────────────────
void UKOGA_Utility_LockOn::ActivateLockOn()
{
    // 가장 적합한 타겟 탐색
    AActor* BestTarget = FindBestTarget();
    if (!BestTarget) return;  // 타겟 없으면 락온 안 함
 
    LockedTarget = BestTarget;
    bIsLockedOn  = true;
 
    // 캐릭터 회전 설정 변경
    ACharacter* OwnerChar = Cast<ACharacter>(GetAvatarCharacter());
    if (OwnerChar)
    {
        // 락온 중: 카메라 Yaw 방향으로 캐릭터 회전 ON
        OwnerChar->bUseControllerRotationYaw = true;
        // 락온 중: 이동 방향 자동 회전 OFF
        OwnerChar->GetCharacterMovement()->bOrientRotationToMovement = false;
    }
 
    // GAS 태그 부여
    ApplyLockOnGameplayTag(true);
 
    // 타이머 시작
    StartCameraUpdate();           // 카메라 회전 (≈60fps)
    StartLockOnDistanceCheck();    // 거리 체크 (0.2초)
}
 
 
// ─────────────────────────────────────────────────────────────────────
// DeactivateLockOn  ―  락온 끄기
// ─────────────────────────────────────────────────────────────────────
void UKOGA_Utility_LockOn::DeactivateLockOn()
{
    bIsLockedOn  = false;
    LockedTarget = nullptr;
 
    // 캐릭터 회전 설정 복구
    ACharacter* OwnerChar = GetAvatarCharacter();
    if (OwnerChar)
    {
        // 락온 해제: 카메라 Yaw 회전 OFF
        OwnerChar->bUseControllerRotationYaw = false;
        // 락온 해제: 이동 방향 자동 회전 ON
        OwnerChar->GetCharacterMovement()->bOrientRotationToMovement = true;
    }
	
    // GAS 태그 제거
    ApplyLockOnGameplayTag(false);
}
 
 
// ─────────────────────────────────────────────────────────────────────
// FindBestTarget  ―  카메라 정면에서 가장 가까운 적 탐색
// ─────────────────────────────────────────────────────────────────────
AActor* UKOGA_Utility_LockOn::FindBestTarget() const
{
    ACharacter* OwnerChar = Cast<ACharacter>(GetAvatarCharacter());
    if (!OwnerChar) return nullptr;
 
	// 콜리전 스피어로 반경 내 Pawn 감지
	TArray<FOverlapResult> OverlapResults;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(SearchRadius);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerChar);  // 자기 자신 제외

	GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		OwnerChar->GetActorLocation(),  // 중심점
		FQuat::Identity,
		ECC_Pawn,                       // Pawn 채널
		Sphere,
		Params
	);

	AActor* BestTarget = nullptr;
	float   BestDistance = FLT_MAX;    // 가장 가까운 거리 추적

	for (auto& Result : OverlapResults)
	{
		AActor* HitActor = Result.GetActor();
		if (!IsValid(HitActor)) continue;

		// "Enemy" 태그 확인
		if (!HitActor->ActorHasTag(FName("Enemy"))) continue;

		float Distance = FVector::Dist(OwnerChar->GetActorLocation(), HitActor->GetActorLocation());

		// 가장 가까운 적 선택
		if (Distance < BestDistance)
		{
			BestDistance = Distance;
			BestTarget   = HitActor;
		}
	}

	return BestTarget;
}
 
 
// ─────────────────────────────────────────────────────────────────────
// IsTargetValid  ―  타겟이 아직 유효한지 확인
// ─────────────────────────────────────────────────────────────────────
bool UKOGA_Utility_LockOn::IsTargetValid() const
{
    // IsValid: 가비지 컬렉션 되었거나 Pending Kill 상태면 false
    if (!IsValid(LockedTarget)) return false;
 
    float Distance = FVector::Dist(
        GetAvatarCharacter()->GetActorLocation(),
        LockedTarget->GetActorLocation()
    );
 
    // SearchRadius의 1.2배까지는 유효 (약간의 여유)
    return Distance <= SearchRadius * 1.2f;
}
 
 
// ─────────────────────────────────────────────────────────────────────
// GetTargetSocketLocation  ―  타겟의 LockOnSocket 위치 반환
// ─────────────────────────────────────────────────────────────────────
FVector UKOGA_Utility_LockOn::GetTargetSocketLocation() const
{
    if (!LockedTarget) return FVector::ZeroVector;
 
    if (ACharacter* TargetChar = Cast<ACharacter>(LockedTarget))
    {
        USkeletalMeshComponent* Mesh = TargetChar->GetMesh();
        // 소켓이 있으면 소켓 위치, 없으면 액터 위치
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
    // 타겟이 사라졌으면 락온 해제
    if (!IsTargetValid())
    {
    	AActor* CurrentTarget = LockedTarget; // 현재 타겟 임시 저장
    	LockedTarget = nullptr;               // 먼저 nullptr로 초기화

    	AActor* NewTarget = FindBestTarget(); // 탐색 (죽은 타겟 제외됨)
    	if (NewTarget && NewTarget != CurrentTarget)
    	{
    		LockedTarget = NewTarget;
    		GEngine->AddOnScreenDebugMessage(12, 2.f, FColor::Cyan, TEXT("[LockOn] Target Switched"));
    	}
    	else
    	{
    		DeactivateLockOn();
    		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
    		GEngine->AddOnScreenDebugMessage(12, 2.f, FColor::Orange, TEXT("[LockOn] No Target Found"));
    	}
    	return;
    }
 
    ACharacter* OwnerChar = Cast<ACharacter>(GetAvatarCharacter());
    if (!OwnerChar) return;
 
    APlayerController* PC = Cast<APlayerController>(OwnerChar->GetController());
    if (!PC) return;
 
    // 카메라 현재 위치
    FVector  CameraLoc;
    FRotator CameraRot;
    PC->GetPlayerViewPoint(CameraLoc, CameraRot);
 
    // 카메라 → 타겟 소켓을 바라보는 목표 회전값 계산
    FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(
        CameraLoc,
        GetTargetSocketLocation()
    );
	
	TargetRot.Pitch = -35.f; // 원하는 하향 각도 (숫자가 작을수록 더 수직으로 내려다봄)
	
    // 현재 회전에서 목표 회전으로 부드럽게 보간
    // RInterpTo: DeltaTime 없이 고정 간격으로 호출되므로 0.016f 사용
    FRotator NewRot = FMath::RInterpTo(
        PC->GetControlRotation(),
        TargetRot,
        0.016f,             // 타이머 간격과 동일
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
    if (!LockedTarget) return;
 
    float Distance = FVector::Dist(
        GetAvatarCharacter()->GetActorLocation(),
        LockedTarget->GetActorLocation()
    );
 
    // 디버그 화면에 현재 거리 표시 (ID 1 = 항상 같은 줄에 덮어씀)
    GEngine->AddOnScreenDebugMessage(1, 0.3f, FColor::Yellow,
        FString::Printf(TEXT("[LockOn] Distance: %.0f / Break: %.0f"), Distance, LockOnBreakDistance));
 
    // 거리 초과 시 락온 자동 해제
    if (Distance >= LockOnBreakDistance)
    {
        DeactivateLockOn();
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
        GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange, TEXT("[LockOn] Deactivated (Distance)"));
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
        true   // 반복
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
void UKOGA_Utility_LockOn::ApplyLockOnGameplayTag(bool bApply) const
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


