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
	// 트레이스 반경
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Attack")
	float TraceRadius = 50.f;
 
	// 디버그 구체 표시 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Attack")
	bool bShowDebug = true;
 
private:
	// 이전 프레임 소켓 위치
	FVector PrevSocketLocation = FVector::ZeroVector;
 
	// 현재 프레임 소켓 위치
	FVector CurrSocketLocation = FVector::ZeroVector;
 
	// 이미 피격된 액터 목록 (중복 타격 방지)
	TArray<AActor*> HittedActors;
};
