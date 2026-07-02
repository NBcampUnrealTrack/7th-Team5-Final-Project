#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "KOBossAttackNotifyState.generated.h"

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

private:
	FVector PrevSocketLocation = FVector::ZeroVector;
	
	TArray<TWeakObjectPtr<AActor>> HittedActors;
};
