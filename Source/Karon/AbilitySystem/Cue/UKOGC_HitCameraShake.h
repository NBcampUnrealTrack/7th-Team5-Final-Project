#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "UKOGC_HitCameraShake.generated.h"


UCLASS()
class KARON_API UUKOGC_HitCameraShake : public UGameplayCueNotify_Static
{
	GENERATED_BODY()
	
public:
	UUKOGC_HitCameraShake();
	
protected:
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;

	UPROPERTY(EditDefaultsOnly, Category = "Camera Shake")
	TSubclassOf<UCameraShakeBase> ShakeClass;

	UPROPERTY(EditDefaultsOnly, Category = "Camera Shake")
	float ShakeScale = 1.0f;
};
