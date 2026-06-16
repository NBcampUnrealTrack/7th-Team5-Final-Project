#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "KOANS_TrackingRotation.generated.h"

UCLASS()
class KARON_API UKOANS_TrackingRotation : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	virtual void NotifyTick(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		float FrameDeltaTime,
		const FAnimNotifyEventReference& EventReference
	) override;
 
protected:
	// 회전 속도 - 값이 낮을수록 느리게 추적
	UPROPERTY(EditAnywhere, Category = "Rotation")
	float RotateSpeed = 90.f;
};
