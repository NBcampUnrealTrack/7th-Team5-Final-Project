#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpringArmComponent.h"
#include "KOSpringArmComponent.generated.h"

class AKOCameraManager;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class KARON_API UKOSpringArmComponent : public USpringArmComponent
{
	GENERATED_BODY()

protected:
	virtual void TickComponent(
		float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	AKOCameraManager* GetOrCacheCameraManager();
	
public:
	UPROPERTY(EditAnywhere)
	float BaseArmLength = 300.f;

	UPROPERTY(EditAnywhere)
	FVector BaseSocketOffset = FVector::ZeroVector;

private:
	UPROPERTY(Transient)
	TObjectPtr<AKOCameraManager> CachedCameraManager = nullptr;
};
