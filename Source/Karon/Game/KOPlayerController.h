#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "GMRouterSubsystem.h"
#include "StructUtils/InstancedStruct.h"
#include "KOPlayerController.generated.h"

struct FInputActionValue;

class UKOInputConfig;
class UInputMappingContext;
class UKOInteractionComponent;
class UKOGridBuildComponent;
class UKOInventoryComponent;
class UKOBuildUIComponent;
class UKOMapUIComponent;
class UKOSkillComponent;
class UKOFactoryCraftWidget;



UCLASS()
class KARON_API AKOPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AKOPlayerController();
	
	UFUNCTION()
	void OnItemReceived(FGameplayTag Channel, const FInstancedStruct& Payload);

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void SetupInputComponent() override;

protected:
	// 기본 입력
	void Input_Move(const FInputActionValue& Value);
	void Input_Look(const FInputActionValue& Value);
	void Input_Interact(const FInputActionValue& Value);
	
	// 어빌리티 입력
	void Input_AbilityPressed(FGameplayTag InputTag);
	void Input_AbilityReleased(FGameplayTag InputTag);

	// 건설 입력 (열기 전용 — 닫기는 Back)
	void Input_OpenBuildMode(const FInputActionValue& Value);

	void Input_BuildConfirm(const FInputActionValue& Value);
	void Input_BuildCancel(const FInputActionValue& Value);

	void Input_BuildToggleDestroy(const FInputActionValue& Value);

	void Input_SelectBuildQuickSlot1(const FInputActionValue& Value);
	void Input_SelectBuildQuickSlot2(const FInputActionValue& Value);
	void Input_SelectBuildQuickSlot3(const FInputActionValue& Value);
	void Input_SelectBuildQuickSlot4(const FInputActionValue& Value);
	void Input_SelectBuildQuickSlot5(const FInputActionValue& Value);
	
	void Input_BuildRotate(const FInputActionValue& Value);
	
	void Input_OpenPlayerMenu(const FInputActionValue& Value);

	void Input_ToggleMap(const FInputActionValue& Value);


private:
	// DefaultIMC ↔ BuildIMC 스왑. 건설 모드 진입/종료(Data.Message.Build.ModeChanged)에 반응.
	void EnterBuildIMC();
	void ExitBuildIMC();
	
	//TryAddItem+ UI 
	void TryAddItemWithUI(FName ItemId,int32 Count);

	// 건설 모드 변경 메시지 수신 → BuildIMC 추가/제거.
	UFUNCTION()
	void OnBuildModeChanged(FGameplayTag Channel, const FInstancedStruct& Payload);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UKOInputConfig> InputConfig;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultIMC;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> BuildIMC;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<UKOInteractionComponent> InteractionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build")
	TObjectPtr<UKOGridBuildComponent> GridBuildComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build")
	TObjectPtr<UKOBuildUIComponent> BuildUIComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UKOInventoryComponent> InventoryComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="KO|Map")
	TObjectPtr<UKOMapUIComponent> MapUIComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill")
	TObjectPtr<UKOSkillComponent> SkillComponent; //TODO : 서브시스템으로 전환 

private:
	bool bBuildIMCActive = false;

	// 건설 모드 변경 메시지 구독 (BuildIMC 관리용).
	FGameplayMessageCallback BuildModeChangedCallback;
	FGameplayMessageHandle   BuildModeChangedHandle;
};
