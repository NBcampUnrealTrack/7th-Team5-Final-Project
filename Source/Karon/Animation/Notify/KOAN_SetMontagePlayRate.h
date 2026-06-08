#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "KOAN_SetMontagePlayRate.generated.h"

UCLASS()
class KARON_API UKOAN_SetMontagePlayRate : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference
	) override;
 
protected:
	// 설정할 재생속도
	UPROPERTY(EditAnywhere, Category = "Montage")
	float PlayRate = 1.f;
};
