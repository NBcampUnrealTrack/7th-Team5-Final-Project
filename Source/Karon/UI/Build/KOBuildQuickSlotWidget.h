#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "GMRouterSubsystem.h"
#include "Utility/Messaging/KOGMSInterface.h"
#include "KOBuildQuickSlotWidget.generated.h"

class UImage;
class UBorder;
class UTexture2D;
class UTextBlock;
class UDragDropOperation;
class UKOBuildUIComponent;
class UKOInventoryComponent;
class UKOItemTooltipWidget;

UENUM(BlueprintType)
enum class EKOQuickSlotBarDisplayMode : uint8
{
	Inventory,
	BuildMode
};

UCLASS()
class KARON_API UKOBuildQuickSlotWidget : public UUserWidget, public IKOGMSInterface
{
	GENERATED_BODY()

public:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UFUNCTION(BlueprintCallable, Category = "Build|QuickSlot")
	void SetupSlot(int32 InSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Build|QuickSlot")
	void RefreshSlot();
	
	UFUNCTION(BlueprintCallable, Category = "Build|QuickSlot")
	void SetDisplayMode(EKOQuickSlotBarDisplayMode InDisplayMode);

	UFUNCTION(BlueprintPure, Category = "Build|QuickSlot")
	EKOQuickSlotBarDisplayMode GetDisplayMode() const { return DisplayMode; }

	bool IsBuildModeVisualEnabled() const{ return DisplayMode == EKOQuickSlotBarDisplayMode::BuildMode; }

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(
		const FGeometry& InGeometry,
		const FDragDropEvent& InDragDropEvent,
		UDragDropOperation* InOperation
	) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build|QuickSlot|Drag")
	FVector2D DragVisualSize = FVector2D(64.f, 64.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build|QuickSlot|Drag")
	float DragVisualOpacity = 0.85f;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build|QuickSlot", meta = (ExposeOnSpawn = "true"))
	int32 SlotIndex = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build|QuickSlot")
	EKOQuickSlotBarDisplayMode DisplayMode = EKOQuickSlotBarDisplayMode::BuildMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build|QuickSlot")
	TObjectPtr<UTexture2D> EmptySlotIcon = nullptr;

	/** SlotIconImage에 강제할 Desired Size (정사각). SetBrushFromTexture가 텍스처 원본 해상도로 ImageSize를 덮어쓰는 문제 방지용. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build|QuickSlot")
	float SlotIconSize = 64.f;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> SlotIconImage;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CountText;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SlotNumberText;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBorder> SelectedFrameBorder;
	
	// 투명도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build|QuickSlot")
	float NormalOpacity = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build|QuickSlot")
	float DepletedOpacity = 0.25f;
	
	UPROPERTY(EditDefaultsOnly, Category = "KO|UI|Tooltip")
	TSubclassOf<UKOItemTooltipWidget> TooltipClass;
	
private:
	UKOBuildUIComponent* GetBuildUIComponent() const;
	UKOInventoryComponent* GetInventoryComponent() const;
	
	FGameplayMessageHandle QuickSlotChangedHandle;
	FGameplayMessageCallback QuickSlotChangedCallback;
	
	FGameplayMessageHandle InventoryChangedHandle;
	FGameplayMessageCallback InventoryChangedCallback;
	
	FGameplayMessageHandle QuickSlotSelectionChangedHandle;
	FGameplayMessageCallback QuickSlotSelectionChangedCallback;

	UFUNCTION()
	void HandleQuickSlotChangedMessage(FGameplayTag Channel, const FInstancedStruct& Payload);

	UFUNCTION()
	void HandleInventoryChangedMessage(FGameplayTag Channel, const FInstancedStruct& Payload);
	
	UFUNCTION()
	void HandleQuickSlotSelectionChangedMessage(FGameplayTag Channel, const FInstancedStruct& Payload);
	
	void RefreshSlotNumber();
	void RefreshSelectedVisual();
};
