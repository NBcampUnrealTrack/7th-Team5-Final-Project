#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "Items/KOItemSlot.h"
#include "KOItemDragDropOperation.generated.h"

class UTexture2D;
class UKOItemDragSource;

UCLASS()
class KARON_API UKOItemDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "KO|DragDrop")
	FKOItemSlot ItemSlot;

	UPROPERTY(BlueprintReadWrite, Category = "KO|DragDrop")
	FText DisplayName;

	UPROPERTY(BlueprintReadWrite, Category = "KO|DragDrop")
	TObjectPtr<UTexture2D> Icon = nullptr;

	/** 드래그 출발지 폴리모픽 어댑터. Extract/Restore만 호출하면 됨. */
	UPROPERTY(BlueprintReadOnly, Category = "KO|DragDrop")
	TObjectPtr<UKOItemDragSource> Source = nullptr;

public:
	static UKOItemDragDropOperation* CreateItemDragOperation(
		UObject* Outer,
		const FKOItemSlot& InItemSlot,
		FText InDisplayName,
		UTexture2D* InIcon,
		const FVector2D& InDragVisualSize,
		float InDragVisualOpacity,
		UKOItemDragSource* InSource = nullptr
	);

	bool HasItem() const
	{
		return ItemSlot.HasItem();
	}

	bool IsFactory() const
	{
		return ItemSlot.Kind == EKOSlotKind::Factory;
	}

	FName GetItemId() const
	{
		return ItemSlot.ItemId;
	}

	int32 GetCount() const
	{
		return ItemSlot.Count;
	}
};
