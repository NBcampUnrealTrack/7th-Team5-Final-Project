#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KOCharacterMovementComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class KARON_API UKOCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
protected:
	UKOCharacterMovementComponent();
	
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	
};
