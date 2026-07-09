#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Utility/Messaging/KOGMSInterface.h"
#include "KOBuildUIComponent.generated.h"

class UKOGridBuildComponent;
class UKOInventoryComponent;
class UKOBuildKeyGuideWidget;
class UKOInGameHUD;

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

	// 두 퀵슬롯 칸의 내용을 교환. 한쪽이 비어 있어도 동작.
	UFUNCTION(BlueprintCallable, Category = "Build|QuickSlot")
	bool SwapBuildQuickSlot(int32 SlotIndexA, int32 SlotIndexB);

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
	
	// 해제 모드
	UFUNCTION(BlueprintCallable, Category = "Build|Action")
	void StartDestroyBuildMode();

	// 해재 모드 : DestroyMode이면 취소, 아니면 진입.
	UFUNCTION(BlueprintCallable, Category = "Build|Action")
	void ToggleDestroyBuildMode();
	// 좌클릭
	UFUNCTION(BlueprintCallable, Category = "Build|Action")
	void ConfirmBuildAction();
	// 우클릭
	UFUNCTION(BlueprintCallable, Category = "Build|Action")
	void CancelBuildAction();
	// F키 상호작용 여부
	UFUNCTION(BlueprintCallable, Category = "Build|Action")
	bool CancelDestroyModeForInteract();
	// 회전
	UFUNCTION(BlueprintCallable, Category = "Build|Action")
	void RotateBuildPreview(int32 Direction);
	
	void SetHUDKeyGuideMode(bool bBuildMode);
	UKOInGameHUD* GetHUDWidget() const;
	
	// 세이브 로드
	UFUNCTION(BlueprintCallable, Category = "Build|QuickSlot")
	void LoadBuildQuickSlotsFromSave(const TArray<FName>& InBuildQuickSlots);

	const TArray<FName>& GetBuildQuickSlotsForSave() const { return BuildQuickSlots; }

private:
	APlayerController* GetOwningPlayerController() const;
	UKOGridBuildComponent* GetGridBuildComponent() const;
	UKOInventoryComponent* GetInventoryComponent() const;
	
	void SetSelectedBuildQuickSlot(int32 NewSlotIndex);
	void ClearSelectedBuildQuickSlot();
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Build|QuickSlot")
	int32 QuickSlotCount = 5;

	UPROPERTY(VisibleInstanceOnly, Category = "Build|QuickSlot")
	TArray<FName> BuildQuickSlots;

	UPROPERTY(VisibleInstanceOnly, Category = "Build|QuickSlot")
	int32 SelectedQuickSlotIndex = INDEX_NONE;
	
	UPROPERTY()
	TObjectPtr<UKOInGameHUD> HUDWidget;
};
