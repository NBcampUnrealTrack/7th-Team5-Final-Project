#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "GMRouterSubsystem.h"
#include "StructUtils/InstancedStruct.h"
#include "KOPlayerController.generated.h"

class USoundMix;
class UInputAction;
struct FInputActionValue;

class UKOInputConfig;
class UInputMappingContext;
class UKOInteractionComponent;
class UKOGridBuildComponent;
class UKOInventoryComponent;
class UKOBuildUIComponent;
class UKOMapUIComponent;
class UKOFactoryCraftWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWeaponCreate);


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
	
	virtual void OnPossess(APawn* InPawn) override;
	
	virtual void UpdateRotation(float DeltaTime) override;

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
	void Input_BuildInventory(const FInputActionValue& Value);
	
	void Input_OpenPlayerMenu(const FInputActionValue& Value);

	// 모든 위젯이 닫혀 있을 때 ESC → PlayerMenu의 Option 탭 열기.
	// (위젯이 열려 있는 상태의 ESC는 CommonUI Back이 처리한다.)
	void Input_OpenOptionMenu();

	void Input_ToggleMap(const FInputActionValue& Value);
	void Input_Weapon(const FInputActionValue& Value);


private:
	// DefaultIMC ↔ BuildIMC 스왑. 건설 모드 진입/종료(Data.Message.Build.ModeChanged)에 반응.
	void EnterBuildIMC();
	void ExitBuildIMC();
	
	//TryAddItem+ UI 
	void TryAddItemWithUI(FName ItemId,int32 Count);

	// 건설 모드 변경 메시지 수신 → BuildIMC 추가/제거.
	UFUNCTION()
	void OnBuildModeChanged(FGameplayTag Channel, const FInstancedStruct& Payload);
	
	// 저장 파일이 있을 경우
	void LoadOrCreateNewGame();
	void GiveStarterItems();
	
public:
	UFUNCTION(BlueprintCallable)
	float GetTimeSinceLastLookInput() const;
	
public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> IAWeapon; 
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UKOInputConfig> InputConfig;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultIMC;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> BuildIMC;

	UPROPERTY(EditDefaultsOnly, Category = "Autio Settings")
	TObjectPtr<USoundMix> DefaultSoundMix;
	
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
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input")
	float LookUpDownRate = 1.25f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input")
	float LookLeftRightRate = 1.25f;

private:
	bool bBuildIMCActive = false;

	// 건설 모드 변경 메시지 구독 (BuildIMC 관리용).
	FGameplayMessageCallback BuildModeChangedCallback;
	FGameplayMessageHandle   BuildModeChangedHandle;
	
public:
	UPROPERTY(BlueprintAssignable)
	FWeaponCreate OnWeaponCreate;
	
private:
	float LastLookInputTime = -1000.f;
};
