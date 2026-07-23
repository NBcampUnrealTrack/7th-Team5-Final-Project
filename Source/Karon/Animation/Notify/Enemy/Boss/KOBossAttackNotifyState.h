#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "KOBossAttackNotifyState.generated.h"

class UKO_HitData;
class UAbilitySystemComponent;
class UGameplayEffect;

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
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Attack")
	TArray<FKOBossAttackEffectData> DamageEffects;
	
	UPROPERTY(EditAnywhere, Instanced, Category = "Boss|Attack")
	TObjectPtr<UKO_HitData> HitData;

private:
	FVector PrevSocketLocation = FVector::ZeroVector;
	
	TArray<TWeakObjectPtr<AActor>> HittedActors;
	
	bool bHitDetected = false;

	// 데미지 직접 적용
	void ApplyDamageToTarget(
		UAbilitySystemComponent* OwnerASC,
		AActor* TargetActor
	);
};
