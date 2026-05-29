#pragma once

#include "CoreMinimal.h"
#include "KOGameplayAbilityBase.h"
#include "KOGA_LockOn.generated.h"

/**
 * 
 */
UCLASS()
class KARON_API UKOGA_LockOn : public UKOGameplayAbilityBase
{
	GENERATED_BODY()
	
public:
	UKOGA_LockOn();

	// ── GAS 오버라이드 ──────────────────────────────────────────────
 
	// 버튼 첫 누름 시 호출 (락온 활성화)
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
 
	// 락온 활성화: 타겟 탐색 → 상태 설정 → 카메라 타이머 시작
	void ActivateLockOn();
 
	// 락온 비활성화: 상태 초기화 → 캐릭터 회전 복구 → 태그 제거
	void DeactivateLockOn();
 
	// ── 타겟 탐색 & 유효성 ──────────────────────────────────────────
 
	// 카메라 정면 기준 가장 적합한 적 반환 (없으면 nullptr)
	AActor* FindBestTarget() const;
 
	// 타겟이 살아있고 락온 반경 내에 있는지 확인
	bool IsTargetValid() const;
 
	// 타겟의 LockOnSocket 위치 반환 (소켓 없으면 액터 위치)
	FVector GetTargetSocketLocation() const;
	
	// ── 카메라 회전 ─────────────────────────────────────────────────
 
	// 0.016s 타이머로 매 프레임 카메라를 타겟 방향으로 보간 회전
	UFUNCTION()
	void UpdateCameraRotation();
 
	void StartCameraUpdate();   // 카메라 타이머 시작
	void StopCameraUpdate();    // 카메라 타이머 정지
 
	// ── 거리 체크 ───────────────────────────────────────────────────
 
	// 0.2s 마다 거리 확인 → 초과 시 자동 락온 해제
	UFUNCTION()
	void CheckLockOnDistance();
 
	void StartLockOnDistanceCheck();  // 거리 체크 타이머 시작
	void StopLockOnDistanceCheck();   // 거리 체크 타이머 정지
 
	// ── GAS 태그 ────────────────────────────────────────────────────
 
	// ASC에 State.Character.LockOn 태그를 추가/제거
	void ApplyLockOnGameplayTag(bool bApply) const;
 
	// ── 에디터 설정 ─────────────────────────────────────────────────
 
	// 락온 탐색 반경 (cm 단위, 기본 15m)
	UPROPERTY(EditDefaultsOnly, Category = "LockOn")
	float SearchRadius = 1500.f;
 
	// 카메라가 타겟 쪽으로 회전하는 보간 속도
	UPROPERTY(EditDefaultsOnly, Category = "LockOn")
	float CameraInterpSpeed = 8.f;
 
	// 이 거리 이상 멀어지면 락온 자동 해제 (SearchRadius보다 커야 함)
	UPROPERTY(EditDefaultsOnly, Category = "LockOn")
	float LockOnBreakDistance = 1800.f;
 
	// 타겟 캐릭터에서 카메라가 바라볼 소켓 이름
	UPROPERTY(EditDefaultsOnly, Category = "LockOn")
	FName TargetSocketName = FName("LockOnSocket");
 
	// ── 런타임 상태 (저장하지 않음) ────────────────────────────────
 
	UPROPERTY()
	AActor* LockedTarget = nullptr;   // 현재 락온된 타겟
 
	bool bIsLockedOn = false;         // 락온 활성화 여부
 
	FTimerHandle CameraUpdateTimerHandle;       // 카메라 회전 타이머
	FTimerHandle LockOnDistanceTimerHandle;     // 거리 체크 타이머
};
