#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "KOGC_BossAttackCameraShake.generated.h"

UCLASS()
class KARON_API UKOGC_BossAttackCameraShake : public UGameplayCueNotify_Static
{
	GENERATED_BODY()
 
public:
	UKOGC_BossAttackCameraShake();
 
	virtual bool OnExecute_Implementation(
		AActor* MyTarget,
		const FGameplayCueParameters& Parameters
	) const override;
 
	virtual bool OnActive_Implementation(
		AActor* MyTarget,
		const FGameplayCueParameters& Parameters
	) const override;
 
private:
	bool PlayCameraShake(AActor* MyTarget) const;
 
protected:
	UPROPERTY(EditAnywhere, Category = "CameraShake")
	TSubclassOf<UCameraShakeBase> ShakeClass;
 
	UPROPERTY(EditAnywhere, Category = "CameraShake")
	float Scale = 1.f;
};
