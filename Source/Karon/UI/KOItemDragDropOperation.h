#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "Items/KOItemSlot.h"
#include "KOItemDragDropOperation.generated.h"

class UTexture2D;
class UKOInventoryComponent;
class UKOFactoryProcessorComponent;

UCLASS()
class KARON_API UKOItemDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "KO|DragDrop")
	FKOItemSlot ItemSlot;

	UPROPERTY(BlueprintReadOnly, Category = "KO|DragDrop")
	int32 SourceSlotIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "KO|DragDrop")
	TObjectPtr<UKOInventoryComponent> SourceInventoryComponent = nullptr;

	/** 출력 슬롯에서 드래그한 경우 세팅. 인벤토리 드롭 시 TryExtractItem 호출 대상. */
	UPROPERTY(BlueprintReadOnly, Category = "KO|DragDrop")
	TObjectPtr<UKOFactoryProcessorComponent> SourceProcessor = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Build|Drag")
	FText DisplayName;

	UPROPERTY(BlueprintReadWrite, Category = "Build|Drag")
	TObjectPtr<UTexture2D> Icon = nullptr;
	
public:
	// 드래그 시작 함수
	static UKOItemDragDropOperation* CreateItemDragOperation(
		UObject* Outer,
		const FKOItemSlot& InItemSlot,
		FText InDisplayName,
		UTexture2D* InIcon,
		const FVector2D& InDragVisualSize,
		float InDragVisualOpacity,
		int32 InSourceSlotIndex = INDEX_NONE,
		UKOInventoryComponent* InSourceInventoryComponent = nullptr,
		UKOFactoryProcessorComponent* InSourceProcessor = nullptr
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
