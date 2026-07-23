#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "KOCameraManager.generated.h"

class UKOCameraModifier;

UCLASS()
class KARON_API AKOCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()
	
public:
	AKOCameraManager();

	UFUNCTION(BlueprintCallable)
	FRotator GetModifiedControlRotation(float DeltaTime, FRotator BaseRotation) const;

	UFUNCTION(BlueprintCallable)
	float GetModifiedArmLength(float DeltaTime, float BaseArmLength) const;

	UFUNCTION(BlueprintCallable)
	float GetModifiedFOV(float DeltaTime, float BaseFOV) const;
	
	UFUNCTION(BlueprintCallable)
	FVector GetModifiedBoomOffset(float DeltaTime, FVector BaseOffset) const;

	UFUNCTION(BlueprintCallable)
	void InitModifiersWithCharacter(ACharacter* InCharacter);
protected:
	virtual void BeginPlay() override; 

public:
	UPROPERTY(EditDefaultsOnly, Instanced)
	TArray<TObjectPtr<UKOCameraModifier>> Modifiers;
};
