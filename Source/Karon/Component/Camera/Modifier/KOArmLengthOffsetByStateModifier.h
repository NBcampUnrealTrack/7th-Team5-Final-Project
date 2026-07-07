#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "KOCameraModifier.h"
#include "KOArmLengthOffsetByStateModifier.generated.h"

USTRUCT(BlueprintType)
struct FKOArmFramingPreset
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Framing")
	FGameplayTag StateTag;

	UPROPERTY(EditDefaultsOnly, Category = "Framing")
	float ArmLength = 90.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Framing")
	float FOV = 300.f;

	UPROPERTY(EditDefaultsOnly, Category = "Framing")
	FVector SocketOffset = FVector::ZeroVector;
	
	UPROPERTY(EditDefaultsOnly, Category = "Framing")
	int32 Priority = 0;
};

UCLASS()
class KARON_API UKOArmLengthOffsetByStateModifier : public UKOCameraModifier
{
	GENERATED_BODY()

protected:
	virtual void ProcessArmLength_Implementation(float DeltaTime, float& InOutArmLength) override;
	virtual void ProcessBoomOffset_Implementation(float DeltaTime, FVector& InOutOffset) override;
	
	virtual void ProcessFOV_Implementation(float DeltaTime, float& InOutFOV) override;

private:
	const FKOArmFramingPreset* FindActivePreset() const;	

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Framing")
	TArray<FKOArmFramingPreset> Presets;
	
	UPROPERTY(EditDefaultsOnly, Category = "Framing")
	float DefaultArmLength = 300.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Framing")
	float DefaultFOV = 90.f;

	UPROPERTY(EditDefaultsOnly, Category = "Framing")
	FVector DefaultSocketOffset = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Framing")
	float InterpSpeed = 6.f;
};
