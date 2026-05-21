// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/KOItemTypes.h"
#include "KOInventorySlotWidget.generated.h"

class UTexture2D;

UCLASS(Abstract, BlueprintType, Blueprintable)
class KARON_API UKOInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="KO|UI|Inventory")
	void SetSlotData(const FKOItemSlot& InSlot);

	UFUNCTION(BlueprintPure, Category="KO|UI|Inventory")
	const FKOItemSlot& GetSlotData() const;

protected:
	UPROPERTY(BlueprintReadOnly, Category="KO|UI|Inventory")
	FKOItemSlot SlotData;

	UPROPERTY(BlueprintReadOnly, Category="KO|UI|Inventory")
	FText ItemDisplayName;

	UPROPERTY(BlueprintReadOnly, Category="KO|UI|Inventory")
	TObjectPtr<UTexture2D> ItemIcon;

	UFUNCTION(BlueprintImplementableEvent, Category="KO|UI|Inventory")
	void BP_OnSlotDataSet(const FKOItemSlot& NewSlot);
};
