// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/KOItemSlot.h"
#include "KOFactorySlotWidget.generated.h"

class UImage;
class UTextBlock;
class UTexture2D;
class UDragDropOperation;
class UKOEnergyProducerComponent;
class UKOFactoryProcessorComponent;
class UKOItemTooltipWidget;
class UKOInventoryComponent;

UENUM(BlueprintType)
enum class EKOFactorySlotMode : uint8
{
    Fuel,
    ProcessorInput,
    ProcessorOutput,
};

UCLASS(Abstract, BlueprintType, Blueprintable)
class KARON_API UKOFactorySlotWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void SetupFuelSlot(UKOEnergyProducerComponent* InProducer);
    void SetupInputSlot(UKOFactoryProcessorComponent* InProcessor, FName InItemId);
    void SetupOutputSlot(UKOFactoryProcessorComponent* InProcessor, FName InItemId);

    void RefreshFromComponent();
    
    bool TryMoveInventorySlotToThis(UKOInventoryComponent* Inventory, int32 SlotIndex, const FKOItemSlot& InSlot);

protected:
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual void   NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
    virtual bool   NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UImage> IconImage;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> CountText;

    UPROPERTY(EditDefaultsOnly, Category = "KO|UI|Factory")
    TObjectPtr<UTexture2D> EmptySlotIcon;

    UPROPERTY(EditAnywhere, Category = "KO|UI|Factory|Drag")
    FVector2D DragVisualSize = FVector2D(64.f, 64.f);

    UPROPERTY(EditAnywhere, Category = "KO|UI|Factory|Drag")
    float DragVisualOpacity = 0.85f;

    /** 레시피 선택 시 비어있는 슬롯에 ItemId 아이콘을 표시할 때의 불투명도. */
    UPROPERTY(EditDefaultsOnly, Category = "KO|UI|Factory")
    float PreviewOpacity = 0.3f;

    /** IconImage에 강제할 Desired Size (정사각). SetBrushFromTexture가 텍스처 원본 해상도로 ImageSize를 덮어쓰는 문제 방지용. */
    UPROPERTY(EditDefaultsOnly, Category = "KO|UI|Factory")
    float SlotIconSize = 64.f;
    
    UPROPERTY(EditDefaultsOnly, Category = "KO|UI|Tooltip")
    TSubclassOf<UKOItemTooltipWidget> TooltipClass;

private:
    void ApplyVisual(FName ItemId, int32 Count);

    EKOFactorySlotMode Mode = EKOFactorySlotMode::Fuel;
    TWeakObjectPtr<UKOEnergyProducerComponent>   Producer;
    TWeakObjectPtr<UKOFactoryProcessorComponent> Processor;

    /** Input/Output 모드에서 이 슬롯이 표현하는 아이템 ID. Fuel은 사용 안 함. */
    FName SlotItemId = NAME_None;

    /** 마지막 갱신 시점의 카운트 (드래그 페이로드 구성용 캐시) */
    int32 CachedCount = 0;
    
    bool MoveCurrentSlotItemToInventory();

    int32 ExtractCurrentSlotItem(FName& OutItemId);
    void RestoreCurrentSlotItem(FName ItemId, int32 Count);
    
    UKOInventoryComponent* ResolvePlayerInventory() const;
};
