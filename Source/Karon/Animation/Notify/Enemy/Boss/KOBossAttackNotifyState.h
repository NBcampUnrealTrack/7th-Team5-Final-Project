#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "KOBossAttackNotifyState.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;

// ─── 추가 : 노티파이스테이트 전용 데미지 GE 데이터 ───────────
// GA 의존성 없이 노티파이스테이트에서 직접 데미지 적용
USTRUCT(BlueprintType)
struct FKOBossAttackEffectData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> EffectClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Level = 1.f;

	// 보스 AttackPower에 곱할 배율
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float AttackCoefficient = 1.f;
};

UCLASS()
class KARON_API UKOBossAttackNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	virtual void NotifyBegin(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		float TotalDuration,
		const FAnimNotifyEventReference& EventReference
	) override;
 
	virtual void NotifyTick(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		float FrameDeltaTime,
		const FAnimNotifyEventReference& EventReference
	) override;
 
	virtual void NotifyEnd(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference
	) override;
 
protected:
	// 공격 판정용 소캣 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Attack")
	FName AttackSocketName = FName("hand_r");
	
	// 트레이스 반경
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Attack")
	float TraceRadius = 50.f;

	// ─── 추가 : 데미지 GE 배열 ────────────────────────────────
	// 보스 AttackPower × AttackCoefficient로 데미지 계산
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Attack")
	TArray<FKOBossAttackEffectData> DamageEffects;

private:
	FVector PrevSocketLocation = FVector::ZeroVector;
	
	TArray<TWeakObjectPtr<AActor>> HittedActors;

	// 데미지 직접 적용
	void ApplyDamageToTarget(
		UAbilitySystemComponent* OwnerASC,
		AActor* TargetActor
	);
};
