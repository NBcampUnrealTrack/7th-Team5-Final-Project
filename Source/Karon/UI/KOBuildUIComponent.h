#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Messaging/KOGMSInterface.h"
#include "KOBuildUIComponent.generated.h"

class UCommonActivatableWidget;
class UKOGridBuildComponent;
class UKOInventoryComponent;

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

	// 퀵슬롯에 등록할 때 호출
	UFUNCTION(BlueprintCallable, Category = "Build|QuickSlot")
	bool SetBuildQuickSlot(int32 SlotIndex, FName FactoryId);

	// 1~5번 키
	UFUNCTION(BlueprintCallable, Category = "Build|QuickSlot")
	void SelectBuildQuickSlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Build|QuickSlot")
	FName GetBuildQuickSlot(int32 SlotIndex) const;
	
	UFUNCTION(BlueprintPure, Category = "Build|QuickSlot")
	int32 GetQuickSlotCount() const;
	
	UFUNCTION(BlueprintPure, Category = "Build|QuickSlot")
	int32 GetSelectedBuildQuickSlotIndex() const;
	
	UFUNCTION(BlueprintCallable, Category = "Build|QuickSlot")
	void OpenQuickSlotBar();

	UFUNCTION(BlueprintCallable, Category = "Build|QuickSlot")
	void CloseQuickSlotBar();
	
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
	UKOInventoryComponent* GetInventoryComponent() const;
	
	void SetSelectedBuildQuickSlot(int32 NewSlotIndex);
	void ClearSelectedBuildQuickSlot();
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Build|UI")
	TSubclassOf<UCommonActivatableWidget> QuickSlotBarWidgetClass;

	UPROPERTY()
	TObjectPtr<UCommonActivatableWidget> QuickSlotBarWidget;

	UPROPERTY(EditDefaultsOnly, Category = "Build|QuickSlot")
	int32 QuickSlotCount = 5;

	UPROPERTY(VisibleInstanceOnly, Category = "Build|QuickSlot")
	TArray<FName> BuildQuickSlots;

	UPROPERTY(VisibleInstanceOnly, Category = "Build|QuickSlot")
	int32 SelectedQuickSlotIndex = INDEX_NONE;
};