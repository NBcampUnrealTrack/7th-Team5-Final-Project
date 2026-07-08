#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/KOGameplayAbilityBase.h"
#include "KOGA_HitReact.generated.h"


UENUM(BlueprintType)
enum class EHitDirection : uint8
{
	Forward UMETA(DisplayName="Forward"),
	Backward UMETA(DisplayName="Backward"),
	Left UMETA(DisplayName="Left"),
	Right UMETA(DisplayName="Right"),
};

UCLASS()
class KARON_API UKOGA_HitReact : public UKOGameplayAbilityBase
{
	GENERATED_BODY()
	
public:
	UKOGA_HitReact(); 
	
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		 const FGameplayEventData* TriggerEventData
	) override;
	
	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility, bool bWasCancelled
	) override;
	
	//TODO:
	// 피격 감지 
	// 방향 분기 
	// GE (데미지 / 상태이상 등)  + 애니메이션
	// 히트 스톱 Task ? 
	// 넉백 
	// 카메라 쉐이크  + VFX  << 이건 Cue ? 
	// 짧은 무적 후 종료 
	
protected:
	void ExecuteKnockBack(const FGameplayEventData& EventData);

	void RotateTowardsAttacker(const FGameplayEventData& EventData);
	
private:
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
	TSubclassOf<class UGameplayEffect> GE_Invincible; 
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
	float InvincibleDuration = 1.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage")
	TMap<EHitDirection, TObjectPtr<UAnimMontage>> DirectionalMontage;
	
	EHitDirection HitDirection;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KnockBack")
	float KnockBackAmount = 1000.f; 
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitStop", meta = (ToolTip = "히트스톱 사용 여부"))
	bool bShouldHitStop;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitStop", meta = (ToolTip = "히트스톱 지속시간(초)"))
	float HitStopDuration = 0.06f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitStop", meta = (ClampMin = "0.0", ClampMax = "1.0", 
		ToolTip=" 0 = 완전 정지 / 0.05 ~ 0.1 = 슬로우 모션"))
	float HitStopTimeDilation = 0.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitStop", Meta = (DisplayName = "공격자도 함께 정지"))
	bool bAffectInstigator = true; 
	
private:
	FGameplayEventData CachedTriggerEventData;
	
	TWeakObjectPtr<AActor> CachedInstigator;
};
