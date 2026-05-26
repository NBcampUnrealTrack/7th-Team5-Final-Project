#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "KOPlayerController.generated.h"

struct FInputActionValue;

class UKOInputConfig;
class UInputMappingContext;
class UKOInteractionComponent;
class UKOActivatableWidget;

UCLASS()
class KARON_API AKOPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AKOPlayerController();
	
protected:
	virtual void BeginPlay() override;
	
	virtual void SetupInputComponent() override; 
	
protected:
	void Input_Move(const FInputActionValue& Value);
	
	void Input_Look(const FInputActionValue& Value);
	
	void Input_AbilityPressed(FGameplayTag InputTag);

	void Input_AbilityReleased(FGameplayTag InputTag);

	void Input_Interact(const FInputActionValue& Value);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UKOInputConfig> InputConfig;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultIMC;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UKOInteractionComponent> InteractionComponent;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UKOActivatableWidget> RootLayoutClass;

private:
	UPROPERTY()
	TObjectPtr<UKOActivatableWidget> RootLayoutInstance;

	void CreateRootLayout();
};
