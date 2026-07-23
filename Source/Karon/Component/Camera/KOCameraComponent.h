#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "KOCameraComponent.generated.h"


class AKOCameraManager;

UCLASS()
class KARON_API UKOCameraComponent : public UCameraComponent
{
	GENERATED_BODY()

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	AKOCameraManager* GetOrCacheCameraManager();
	
public:
	UPROPERTY(EditAnywhere)
	float BaseFOV = 90.f;
	
private:
	UPROPERTY(Transient)
	TObjectPtr<AKOCameraManager> CachedCameraManager = nullptr;
};
