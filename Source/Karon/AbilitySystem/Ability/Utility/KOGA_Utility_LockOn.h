#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"    
#include "AbilitySystem/Ability/KOGameplayAbilityBase.h"
#include "KOGA_Utility_LockOn.generated.h"

/*
 *	TODO: 유나님 숙제 
	1. 타이머 콜백 내부에서 EndAbility 직접 호출 (크래시 위험)
	2. FindBestTarget 주석과 구현 불일치 (카메라 정면 고려 없음)
	3. UpdateCameraRotation Pitch 하드코딩 (-35.f)
	4. RInterpTo에 DeltaTime 대신 고정값 0.016f 사용 (프레임레이트 종속)
	5. bDeactivatedByInput 플래그로 재진입 차단하는 구조가 불안정
	6. EndAbility의 쿨타임 조건(bWasCancelled) 의도 불명확
	7. IsTargetValid가 거리만 체크하고 액터 생존 여부(HP 등) 미확인
	8. ApplyLockOnGameplayTag가 const인데 ASC 상태를 변경함 (논리적 불일치)
	9. LockedTarget이 TWeakObjectPtr 아닌 raw 포인터 (TObjectPtr 권장)
	10. DeactivateLockOn 중복 호출 가능성 (EndAbility와 InputPressed 양쪽에서 호출)
	11. Enemy 태그 하드코딩 (FName("Enemy")) — GAS 태그 시스템과 불일치
	12. CommitAbility 누락 — 코스트/쿨타임이 어빌리티 활성화 시 적용 안 됨
	13. ActivateLockOn 내 ACharacter* 두번 캐스팅 
	14. GEngine->AddOnScreenDebugMessage 다수 — 출판 코드에 디버그 출력 잔존
	15. ActivateAbility 내 Character 미사용 
	16. 주석 투머치 Til로 보내자 
 */



UCLASS()
class KARON_API UKOGA_Utility_LockOn : public UKOGameplayAbilityBase
{
	GENERATED_BODY()
	
public:
	UKOGA_Utility_LockOn();

	// ── GAS 오버라이드 ──────────────────────────────────────────────
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	// 어빌리티 종료 시 호출 (타이머 정리 등 클린업)
	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;
	
	// ASC가 InputPressed 신호를 보낼 때 호출됨 (어빌리티가 Active 상태일 때)
	// 어빌리티 Active 상태에서 버튼 재누름 시 호출 (락온 해제)
	virtual void InputPressed(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;

private:
	// ── 락온 핵심 로직 ──────────────────────────────────────────────
	void ActivateLockOn();
	void DeactivateLockOn();
 
	// ── 타겟 탐색 & 유효성 ──────────────────────────────────────────
	AActor* FindBestTarget() const;
	bool IsTargetValid() const;
	FVector GetTargetSocketLocation() const;
	
	// ── 카메라 회전 ─────────────────────────────────────────────────
	UFUNCTION()
	void UpdateCameraRotation();
	
	bool CameraTick(float DeltaTime);
	
	void StartCameraUpdate();   // 카메라 타이머 시작
	void StopCameraUpdate();    // 카메라 타이머 정지
 
	// ── 거리 체크 ───────────────────────────────────────────────────
	UFUNCTION()
	void CheckLockOnDistance();
 
	void StartLockOnDistanceCheck();  // 거리 체크 타이머 시작
	void StopLockOnDistanceCheck();   // 거리 체크 타이머 정지
 
	// ── GAS 태그 ────────────────────────────────────────────────────
	void ApplyLockOnGameplayTag(bool bApply); 
 
	// ── 에디터 설정 ─────────────────────────────────────────────────
	UPROPERTY(EditDefaultsOnly, Category = "LockOn")
	float SearchRadius = 1500.f;
 
	// 카메라가 타겟 쪽으로 회전하는 보간 속도
	UPROPERTY(EditDefaultsOnly, Category = "LockOn")
	float CameraInterpSpeed = 8.f;
 
	// 이 거리 이상 멀어지면 락온 자동 해제 (SearchRadius보다 커야 함)
	UPROPERTY(EditDefaultsOnly, Category = "LockOn")
	float LockOnBreakDistance = 1800.f;
 
	
	UPROPERTY(EditDefaultsOnly, Category = "LockOn")
	FName TargetSocketName = FName("LockOnSocket");
 
	UPROPERTY(EditDefaultsOnly, Category = "LockOn")
	float LockOnCameraPitch = -35.f;
 
	// 보스 타겟 락온 시 사용할 별도 카메라 Pitch (보스는 덩치가 크므로 더 위에서 봄)
	UPROPERTY(EditDefaultsOnly, Category = "LockOn")
	float BossLockOnCameraPitch = 10.f;
	
	
	// ── 소프트 락온(리시) 설정 ──────────────────────────────────
	// 타겟 정면(앵커) 기준, 마우스로 벗어날 수 있는 좌우 최대 각도
	UPROPERTY(EditDefaultsOnly, Category = "LockOn|Soft")
	float LeashYaw = 40.f;

	// 앵커 기준 상하 최대 각도
	UPROPERTY(EditDefaultsOnly, Category = "LockOn|Soft")
	float LeashPitch = 20.f;

	// 마우스에서 손 뗀 뒤 앵커로 복귀하는 보간 속도 (CameraInterpSpeed보다 느리게)
	UPROPERTY(EditDefaultsOnly, Category = "LockOn|Soft")
	float RecenterInterpSpeed = 4.5f;

	// 이 시간(초) 이상 마우스 입력이 없으면 "복귀 모드"로 전환
	UPROPERTY(EditDefaultsOnly, Category = "LockOn|Soft")
	float ReactivateDelay = 1.0f;
	
	
	// 보스 락온 시 카메라를 추가로 뒤로 빼는 거리 (cm)
	UPROPERTY(EditDefaultsOnly, Category = "LockOn")
	float BossLockOnExtraArmLength = 350.f;
	
	// 이 거리보다 타겟이 가까워지면 카메라를 뒤로 빼기 시작 (cm)
	UPROPERTY(EditDefaultsOnly, Category = "LockOn")
	float CloseDistanceThreshold = 400.f;

	// 가장 가까울 때 추가로 빼는 최대 거리 (cm)
	UPROPERTY(EditDefaultsOnly, Category = "LockOn")
	float MaxCloseExtraArmLength = 250.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "LockOn")
	FVector LockOnCameraOffset = FVector(200.f, 0.f, 0.f); // Y=오른쪽, Z=위

	
	FVector DefaultCameraOffset = FVector::ZeroVector; // 락온 해제 시 복구용
	float DefaultArmLength = 0.f; 
	
	// 하드코딩 방지를 위한 액터 태그 변수 노출
	UPROPERTY(EditDefaultsOnly, Category = "LockOn")
	FName EnemyActorTag = FName("Enemy");

	UPROPERTY(EditDefaultsOnly, Category = "LockOn")
	FName BossActorTag = FName("Boss");
	
	// 디버그 메시지 출력 여부 제어
	UPROPERTY(EditDefaultsOnly, Category = "LockOn")
	bool bShowDebugMessages = true;
 
	// ── 런타임 상태 ─────────────────────────────────────────────────
	// 메모리 누수 및 크래시 방지를 위해 약참조(TWeakObjectPtr) 사용
	UPROPERTY()
	TWeakObjectPtr<AActor> LockedTarget = nullptr;
 
	bool bIsLockedOn = false;
	float LockOnActivationTime = 0.f;
	
	//FTimerHandle CameraUpdateTimerHandle;
	FTSTicker::FDelegateHandle CameraTickHandle; 
	FTimerHandle LockOnDistanceTimerHandle;
};
