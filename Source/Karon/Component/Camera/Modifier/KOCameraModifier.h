#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "KOCameraModifier.generated.h"

class AKOCharacterBase;
UCLASS(Abstract, Blueprintable, EditInlineNew)
class KARON_API UKOCameraModifier : public UObject
{
	GENERATED_BODY()
public:
	// 현재 이 모디파이어를 적용 유무 
	UFUNCTION(BlueprintNativeEvent)
	bool IsModifierActive() const;
	virtual bool IsModifierActive_Implementation() const;

	// 카메라 회전 
	UFUNCTION(BlueprintNativeEvent)
	void ProcessControlRotation(float DeltaTime, FRotator& InOutRotation);
	virtual void ProcessControlRotation_Implementation(float DeltaTime, FRotator& InOutRotation) {}

	// 카메라 암 길이 
	UFUNCTION(BlueprintNativeEvent)
	void ProcessArmLength(float DeltaTime, float& InOutArmLength);
	virtual void ProcessArmLength_Implementation(float DeltaTime, float& InOutArmLength) {}

	// FOV 
	UFUNCTION(BlueprintNativeEvent)
	void ProcessFOV(float DeltaTime, float& InOutFOV);
	virtual void ProcessFOV_Implementation(float DeltaTime, float& InOutFOV) {}

	// 암 소켓 오프셋 
	UFUNCTION(BlueprintNativeEvent)
	void ProcessBoomOffset(float DeltaTime, FVector& InOutOffset);
	virtual void ProcessBoomOffset_Implementation(float DeltaTime, FVector& InOutOffset) {}

public:
	UPROPERTY(Transient, BlueprintReadOnly)
	TObjectPtr<AKOCharacterBase> OwningCharacter;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FGameplayTag ActivationTag; 
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bEnabled = true;
	
	UPROPERTY(EditDefaultsOnly)
	int32 Priority = 0;
};
