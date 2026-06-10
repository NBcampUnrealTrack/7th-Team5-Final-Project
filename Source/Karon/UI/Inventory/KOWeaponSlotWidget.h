#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KOWeaponSlotWidget.generated.h"

class UKOItemDragDropOperation;
class UImage;
class UTextBlock;
class UKOInventoryComponent;
class UKOItemTooltipWidget;

UCLASS()
class KARON_API UKOWeaponSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "WeaponSlot")
	FName EquippedWeaponId = NAME_None;
	
	UFUNCTION(BlueprintCallable, Category = "KO|WeaponSlot")
	void SetInventoryComponent(UKOInventoryComponent* InInventory);

	/** 장착 해제: 장착 무기를 인벤토리로 되돌림 */
	bool UnequipWeapon();

protected:
	virtual void NativeConstruct() override;
	
	virtual bool NativeOnDrop(
		const FGeometry& InGeometry,
		const FDragDropEvent& InDragDropEvent,
		UDragDropOperation* InOperation
	) override;
	
	virtual FReply NativeOnMouseButtonDown(
	   const FGeometry& InGeometry,
	   const FPointerEvent& InMouseEvent
   ) override;

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> WeaponIconImage;
	
	UPROPERTY(BlueprintReadOnly, Category = "KO|WeaponSlot")
	TObjectPtr<UKOInventoryComponent> InventoryComponent;
	
	UPROPERTY(EditDefaultsOnly, Category = "KO|Tooltip")
	TSubclassOf<UKOItemTooltipWidget> ItemTooltipWidgetClass;

private:
	void ResolveInventoryComponentIfNeeded();
	
	void RefreshVisual();
	
	void RefreshTooltip();
};