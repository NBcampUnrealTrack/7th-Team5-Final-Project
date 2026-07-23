#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "KOAN_BossLaunchForward.generated.h"

UCLASS()
class KARON_API UKOAN_BossLaunchForward : public UAnimNotify
{
	GENERATED_BODY()
 
public:
	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference
	) override;
 
protected:
	UPROPERTY(EditAnywhere, Category = "Launch")
	float LaunchSpeed = 1000.f;
	
	UPROPERTY(EditAnywhere, Category = "Launch")
	float LaunchUpSpeed = 0.f;
	
	UPROPERTY(EditAnywhere, Category = "Launch")
	bool bOverrideXY = true;
	
	UPROPERTY(EditAnywhere, Category = "Launch")
	bool bOverrideZ = true;
	
	UPROPERTY(EditAnywhere, Category = "Launch", meta = (ClampMin = "0.0"))
	float FrictionRestoreDelay = 0.3f;
};
