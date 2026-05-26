#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Messaging/KOGMSInterface.h"
#include "KOBuildUIComponent.generated.h"

class UCommonActivatableWidget;
class UKOGridBuildComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class KARON_API UKOBuildUIComponent : public UActorComponent, public IKOGMSInterface
{
	GENERATED_BODY()

public:
	UKOBuildUIComponent();

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Build|UI")
	void OpenBuildMenu();

	UFUNCTION(BlueprintCallable, Category = "Build|UI")
	void CloseBuildMenu();
	// B키
	UFUNCTION(BlueprintCallable, Category = "Build|UI")
	void ToggleBuildMenu();

	UFUNCTION(BlueprintCallable, Category = "Build|UI")
	bool IsBuildMenuOpen() const;
	
	UFUNCTION(BlueprintCallable, Category = "Build|UI")
	void OpenBuildAssignMenu();

	UFUNCTION(BlueprintCallable, Category = "Build|UI")
	void CloseBuildAssignMenu();
	// I키
	UFUNCTION(BlueprintCallable, Category = "Build|UI")
	void ToggleBuildAssignMenu();

	UFUNCTION(BlueprintCallable, Category = "Build|UI")
	bool IsBuildAssignMenuOpen() const;

	// UI에서 설비를 퀵슬롯에 등록할 때 호출
	UFUNCTION(BlueprintCallable, Category = "Build|QuickSlot")
	bool SetBuildQuickSlot(int32 SlotIndex, FName FactoryId);

	// 1~5번 키
	UFUNCTION(BlueprintCallable, Category = "Build|QuickSlot")
	void SelectBuildQuickSlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Build|QuickSlot")
	FName GetBuildQuickSlot(int32 SlotIndex) const;
	
	UFUNCTION(BlueprintPure, Category = "Build|QuickSlot")
	int32 GetQuickSlotCount() const;
	
	// X 키
	UFUNCTION(BlueprintCallable, Category = "Build|Action")
	void StartDestroyBuildMode();
	// 좌클릭
	UFUNCTION(BlueprintCallable, Category = "Build|Action")
	void ConfirmBuildAction();
	// 우클릭
	UFUNCTION(BlueprintCallable, Category = "Build|Action")
	void CancelBuildAction();
	// ESC
	UFUNCTION(BlueprintCallable, Category = "Build|Action")
	void EscapeBuildAction();

private:
	APlayerController* GetOwningPlayerController() const;
	UKOGridBuildComponent* GetGridBuildComponent() const;
	
	void OpenQuickSlotBar();
	void CloseQuickSlotBar();

private:
	UPROPERTY(EditDefaultsOnly, Category = "Build|UI")
	TSubclassOf<UCommonActivatableWidget> QuickSlotBarWidgetClass;

	UPROPERTY()
	TObjectPtr<UCommonActivatableWidget> QuickSlotBarWidget;

	UPROPERTY(EditDefaultsOnly, Category = "Build|UI")
	TSubclassOf<UCommonActivatableWidget> BuildAssignMenuWidgetClass;

	UPROPERTY()
	TObjectPtr<UCommonActivatableWidget> BuildAssignMenuWidget;

	UPROPERTY(EditDefaultsOnly, Category = "Build|QuickSlot")
	int32 QuickSlotCount = 5;

	UPROPERTY(VisibleInstanceOnly, Category = "Build|QuickSlot")
	TArray<FName> BuildQuickSlots;
};