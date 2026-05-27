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
	// 기본 입력
	void Input_Move(const FInputActionValue& Value);
	void Input_Look(const FInputActionValue& Value);
	void Input_Interact(const FInputActionValue& Value);
	
	// 어빌리티 입력
	void Input_AbilityPressed(FGameplayTag InputTag);
	void Input_AbilityReleased(FGameplayTag InputTag);

	// 건설 입력
	void Input_ToggleBuildMode(const FInputActionValue& Value);
	void Input_ToggleBuildAssignMenu(const FInputActionValue& Value);

	void Input_BuildConfirm(const FInputActionValue& Value);	
	void Input_BuildCancel(const FInputActionValue& Value);
	void Input_BuildEscape(const FInputActionValue& Value);
	
	void Input_BuildToggleDestroy(const FInputActionValue& Value);
	
	void Input_SelectBuildQuickSlot1(const FInputActionValue& Value);
	void Input_SelectBuildQuickSlot2(const FInputActionValue& Value);

private:
	// DefaultIMC ↔ BuildIMC 스왑 + GridBuildComponent 진입/종료
	void EnterBuildIMC();
	void ExitBuildIMC();
	
	void CreateRootLayout();

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
	
	

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UKOActivatableWidget> RootLayoutClass;

private:
	UPROPERTY()
	TObjectPtr<UKOActivatableWidget> RootLayoutInstance;

	bool bBuildIMCActive = false;
};
