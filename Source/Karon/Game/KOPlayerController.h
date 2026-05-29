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
class UKOInventoryComponent;
class UKOActivatableWidget;
class UKOBuildUIComponent;

// TODO: 
// 공장 <-> 전투 맵이 분리되면 컨트롤러를 두개로 관리하는것도 나쁘지않을듯? 
//		맵이 분리되는 거면 굳이 이걸 전투 쪽에서도 들고 있을 필요는 없어서 
// 공장 컨트롤러 <-> 전투 컨트롤러 
// 공통 로직은 이동 화면 전환 정도 ? 
// 이거 베이스로 올려버리면 깔 - 끔 할 듯 


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

	void Input_BuildToggleDestroy(const FInputActionValue& Value);

	void Input_SelectBuildQuickSlot1(const FInputActionValue& Value);
	void Input_SelectBuildQuickSlot2(const FInputActionValue& Value);
	void Input_SelectBuildQuickSlot3(const FInputActionValue& Value);
	void Input_SelectBuildQuickSlot4(const FInputActionValue& Value);
	void Input_SelectBuildQuickSlot5(const FInputActionValue& Value);

	// 인벤토리 입력
	void Input_ToggleInventory(const FInputActionValue& Value);
	
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UKOInventoryComponent> InventoryComponent;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UKOActivatableWidget> RootLayoutClass;

private:
	UPROPERTY()
	TObjectPtr<UKOActivatableWidget> RootLayoutInstance;

	bool bBuildIMCActive = false;
};
