#pragma once

#include "CoreMinimal.h"
#include "KOCameraModifier.h"
#include "KOSpeedFOVModifier.generated.h"

UCLASS()
class KARON_API UKOSpeedFOVModifier : public UKOCameraModifier
{
	GENERATED_BODY()
protected:
	virtual void ProcessFOV_Implementation(float DeltaTime, float& InOutFOV) override;

public:
	UPROPERTY(EditDefaultsOnly, Category = "SpeedFOV")
	float ReferenceSpeed = 600.f;   

	UPROPERTY(EditDefaultsOnly, Category = "SpeedFOV")
	float MaxFOVBoost = 15.f;

	UPROPERTY(EditDefaultsOnly, Category = "SpeedFOV")
	UCurveFloat* SpeedToAlphaCurve;

	UPROPERTY(EditDefaultsOnly, Category = "SpeedFOV")
	float InterpSpeed = 6.f;

private:
	float CurrentBoost = 0.f;
};
