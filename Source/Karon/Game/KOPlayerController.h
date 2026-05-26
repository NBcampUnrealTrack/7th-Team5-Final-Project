#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "KOPlayerController.generated.h"

struct FInputActionValue;

class UKOInputConfig;
class UInputMappingContext;
class UKOInteractionComponent;
class UKOGridBuildComponent;
class UKOActivatableWidget;
class UKOBuildUIComponent;

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

	void Input_ToggleBuildMode(const FInputActionValue& Value);

	void Input_BuildConfirm(const FInputActionValue& Value);

	void Input_BuildToggleDestroy(const FInputActionValue& Value);

private:
	// DefaultIMC ↔ BuildIMC 스왑 + GridBuildComponent 진입/종료
	void EnterBuildIMC();
	void ExitBuildIMC();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UKOInputConfig> InputConfig;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultIMC;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> BuildIMC;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UKOInteractionComponent> InteractionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UKOGridBuildComponent> GridBuildComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UKOBuildUIComponent> BuildUIComponent;

	// BuildMenu UI 도입 전, B 키 진입 시 즉시 사용할 디버그 FactoryId
	UPROPERTY(EditDefaultsOnly, Category = "Build")
	FName DebugBuildFactoryId = NAME_None;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UKOActivatableWidget> RootLayoutClass;

private:
	UPROPERTY()
	TObjectPtr<UKOActivatableWidget> RootLayoutInstance;

	bool bBuildIMCActive = false;

	void CreateRootLayout();
};
