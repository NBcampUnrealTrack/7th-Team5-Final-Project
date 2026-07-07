#pragma once

#include "CoreMinimal.h"
#include "KOCameraModifier.h"
#include "KOYawFollowModifer.generated.h"


UCLASS()
class KARON_API UKOYawFollowModifer : public UKOCameraModifier
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, Category = "YawFollow")
	float FollowInterpSpeed = 5.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "YawFollow")
	float ReactivateDelay = 1.0f;

protected:
	virtual bool IsModifierActive_Implementation() const override;
	virtual void ProcessControlRotation_Implementation(float DeltaTime, FRotator& InOutRotation) override;
};
