#include "UI/Build/KOBuildQuickSlotWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Items/KOItemLibrary.h"
#include "Items/KOItemSlot.h"
#include "UI/Inventory/KOItemDragDropOperation.h"
#include "UI/Build/KOBuildUIComponent.h"
#include "UI/Inventory/KOItemDragSource.h"

#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "StructUtils/InstancedStruct.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "Component/Inventory/KOInventoryComponent.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Utility/Messaging/KOMessageTypes.h"

void UKOBuildQuickSlotWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	RefreshSlotNumber();
	RefreshSlot();
}

void UKOBuildQuickSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	QuickSlotChangedCallback.BindDynamic(
		this,
		&UKOBuildQuickSlotWidget::HandleQuickSlotChangedMessage
	);

	QuickSlotChangedHandle = Subscribe(
		KOGameplayTags::Data_Message_Build_QuickSlotChanged,
		QuickSlotChangedCallback
	);
	
	InventoryChangedCallback.BindDynamic(
		this,
		&UKOBuildQuickSlotWidget::HandleInventoryChangedMessage
	);

	InventoryChangedHandle = Subscribe(
		KOGameplayTags::Data_Message_Inventory_Changed,
		InventoryChangedCallback
	);
	
	QuickSlotSelectionChangedCallback.BindDynamic(
		this,
		&UKOBuildQuickSlotWidget::HandleQuickSlotSelectionChangedMessage
	);

	QuickSlotSelectionChangedHandle = Subscribe(
		KOGameplayTags::Data_Message_Build_QuickSlotSelectionChanged,
		QuickSlotSelectionChangedCallback
	);

	RefreshSelectedVisual();
	RefreshSlot();
}

void UKOBuildQuickSlotWidget::NativeDestruct()
{
	Unsubscribe(QuickSlotChangedHandle);
	QuickSlotChangedHandle = FGameplayMessageHandle();
	QuickSlotChangedCallback.Clear();
	
	Unsubscribe(InventoryChangedHandle);
	InventoryChangedHandle = FGameplayMessageHandle();
	InventoryChangedCallback.Clear();
	
	Unsubscribe(QuickSlotSelectionChangedHandle);
	QuickSlotSelectionChangedHandle = FGameplayMessageHandle();
	QuickSlotSelectionChangedCallback.Clear();

	Super::NativeDestruct();
}

UKOBuildUIComponent* UKOBuildQuickSlotWidget::GetBuildUIComponent() const
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return nullptr;
	}

	return PC->FindComponentByClass<UKOBuildUIComponent>();
}

UKOInventoryComponent* UKOBuildQuickSlotWidget::GetInventoryComponent() const
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return nullptr;
	}

	if (UKOInventoryComponent* InventoryComponent = PC->FindComponentByClass<UKOInventoryComponent>())
	{
		return InventoryComponent;
	}

	if (APawn* Pawn = PC->GetPawn())
	{
		return Pawn->FindComponentByClass<UKOInventoryComponent>();
	}

	return nullptr;
}

void UKOBuildQuickSlotWidget::SetupSlot(int32 InSlotIndex)
{
	SlotIndex = InSlotIndex;

	RefreshSlotNumber();
	RefreshSlot();
	RefreshSelectedVisual();
}

void UKOBuildQuickSlotWidget::RefreshSlot()
{
	if (!SlotIconImage)
	{
		return;
	}
	
	auto ApplyEmptyVisual = [this]()
	{
		if (EmptySlotIcon)
		{
			SlotIconImage->SetBrushFromTexture(EmptySlotIcon);
			SlotIconImage->SetDesiredSizeOverride(FVector2D(SlotIconSize, SlotIconSize));
			SlotIconImage->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			// EmptySlotIcon이 없으면 이전 텍스처를 숨겨야 잔상이 남지 않음.
			SlotIconImage->SetBrushFromTexture(nullptr);
			SlotIconImage->SetVisibility(ESlateVisibility::Hidden);
		}

		SlotIconImage->SetRenderOpacity(NormalOpacity);

		if (CountText)
		{
			CountText->SetText(FText::GetEmpty());
			CountText->SetVisibility(ESlateVisibility::Collapsed);
			CountText->SetRenderOpacity(NormalOpacity);
		}
	};

	UKOBuildUIComponent* BuildUIComponent = GetBuildUIComponent();
	if (!BuildUIComponent)
	{
		ApplyEmptyVisual();
		return;
	}

	const FName AssignedFactoryId = BuildUIComponent->GetBuildQuickSlot(SlotIndex);

	if (AssignedFactoryId.IsNone())
	{
		ApplyEmptyVisual();
		return;
	}
	
	UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this);
	if (!LoadSub)
	{
		ApplyEmptyVisual();
		return;
	}

	UTexture2D* Icon = LoadSub->ResolveFactoryIcon(AssignedFactoryId);
	if (!Icon)
	{
		ApplyEmptyVisual();
		return;
	}

	SlotIconImage->SetBrushFromTexture(Icon);
	SlotIconImage->SetDesiredSizeOverride(FVector2D(SlotIconSize, SlotIconSize));
	SlotIconImage->SetVisibility(ESlateVisibility::HitTestInvisible);

	UKOInventoryComponent* InventoryComponent = GetInventoryComponent();

	const bool bHasInventory = InventoryComponent != nullptr;
	const int32 CurrentCount = bHasInventory
		? InventoryComponent->GetCountOf(AssignedFactoryId)
		: 0;

	const bool bDepleted = bHasInventory && CurrentCount <= 0;
	const float TargetOpacity = bDepleted ? DepletedOpacity : NormalOpacity;

	SlotIconImage->SetRenderOpacity(TargetOpacity);

	if (CountText)
	{
		CountText->SetText(FText::AsNumber(CurrentCount));
		CountText->SetVisibility(ESlateVisibility::HitTestInvisible);
		CountText->SetRenderOpacity(TargetOpacity); // 다 사용하면 CountText도 투명하게 하는게 좋을까나..?
	}
}

FReply UKOBuildQuickSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		UKOBuildUIComponent* BuildUIComponent = GetBuildUIComponent();
		if (BuildUIComponent)
		{
			const FName AssignedFactoryId = BuildUIComponent->GetBuildQuickSlot(SlotIndex);
			if (!AssignedFactoryId.IsNone())
			{
				FEventReply Reply = UWidgetBlueprintLibrary::DetectDragIfPressed(
					InMouseEvent,
					this,
					EKeys::LeftMouseButton
				);
				return Reply.NativeReply;
			}
		}
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UKOBuildQuickSlotWidget::NativeOnDragDetected(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	UKOBuildUIComponent* BuildUIComponent = GetBuildUIComponent();
	if (!BuildUIComponent)
	{
		return;
	}

	const FName AssignedFactoryId = BuildUIComponent->GetBuildQuickSlot(SlotIndex);
	if (AssignedFactoryId.IsNone())
	{
		return;
	}

	FKOItemSlot Payload;
	Payload.Kind   = EKOSlotKind::Factory;
	Payload.ItemId = AssignedFactoryId;
	Payload.Count  = 1;

	const FText DisplayName = UKOItemLibrary::GetDisplayName(this, EKOSlotKind::Factory, AssignedFactoryId);
	UTexture2D* Icon        = UKOItemLibrary::GetIcon(this, EKOSlotKind::Factory, AssignedFactoryId);

	UKOItemDragDropOperation* DragOp = UKOItemDragDropOperation::CreateItemDragOperation(
		this,
		Payload,
		DisplayName,
		Icon,
		DragVisualSize,
		DragVisualOpacity,
		nullptr
	);
	if (!DragOp)
	{
		return;
	}

	UKOBuildQuickSlotItemSource* Src = NewObject<UKOBuildQuickSlotItemSource>(DragOp);
	Src->BuildUI   = BuildUIComponent;
	Src->SlotIndex = SlotIndex;
	DragOp->Source = Src;

	OutOperation = DragOp;
}

bool UKOBuildQuickSlotWidget::NativeOnDrop(
	const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation
)
{
	UKOItemDragDropOperation* ItemDragOperation  = Cast<UKOItemDragDropOperation>(InOperation);

	if (!ItemDragOperation )
	{
		return false;
	}

	if (!ItemDragOperation->HasItem())
	{
		return false;
	}

	if (!ItemDragOperation->IsFactory())
	{
		return false;
	}

	const FName FactoryId = ItemDragOperation->GetItemId();

	if (FactoryId.IsNone())
	{
		return false;
	}

	UKOBuildUIComponent* BuildUIComponent = GetBuildUIComponent();
	if (!BuildUIComponent)
	{
		return false;
	}

	// QuickSlot -> QuickSlot: 두 칸 교환.
	if (UKOBuildQuickSlotItemSource* QuickSrc = Cast<UKOBuildQuickSlotItemSource>(ItemDragOperation->Source))
	{
		if (QuickSrc->SlotIndex == SlotIndex)
		{
			return false;
		}
		return BuildUIComponent->SwapBuildQuickSlot(QuickSrc->SlotIndex, SlotIndex);
	}

	// 그 외(예: Inventory에서 출발) -> 단순 할당.
	return BuildUIComponent->SetBuildQuickSlot(SlotIndex, FactoryId);
}

void UKOBuildQuickSlotWidget::HandleQuickSlotChangedMessage(
	FGameplayTag Channel,
	const FInstancedStruct& Payload
)
{
	const FKOBuildQuickSlotChangedMessage* Message = Payload.GetPtr<FKOBuildQuickSlotChangedMessage>();

	if (!Message)
	{
		return;
	}

	if (Message->SlotIndex != SlotIndex)
	{
		return;
	}

	RefreshSlot();
}

void UKOBuildQuickSlotWidget::HandleInventoryChangedMessage(FGameplayTag Channel, const FInstancedStruct& Payload)
{
	const FKOInventoryChangedMessage* Message = Payload.GetPtr<FKOInventoryChangedMessage>();
	if (!Message)
	{
		return;
	}

	UKOBuildUIComponent* BuildUIComponent = GetBuildUIComponent();
	if (!BuildUIComponent)
	{
		return;
	}

	const FName AssignedFactoryId = BuildUIComponent->GetBuildQuickSlot(SlotIndex);

	if (AssignedFactoryId.IsNone())
	{
		return;
	}

	if (Message->ItemId != AssignedFactoryId)
	{
		return;
	}

	RefreshSlot();
}

void UKOBuildQuickSlotWidget::HandleQuickSlotSelectionChangedMessage(FGameplayTag Channel,
	const FInstancedStruct& Payload)
{
	const FKOBuildQuickSlotSelectionChangedMessage* Message =
		Payload.GetPtr<FKOBuildQuickSlotSelectionChangedMessage>();

	if (!Message)
	{
		return;
	}

	if (Message->PreviousSlotIndex != SlotIndex &&
		Message->NewSlotIndex != SlotIndex)
	{
		return;
	}

	RefreshSelectedVisual();
}

void UKOBuildQuickSlotWidget::RefreshSelectedVisual()
{
	if (!SelectedFrameBorder)
	{
		return;
	}

	const UKOBuildUIComponent* BuildUIComponent = GetBuildUIComponent();
	if (!BuildUIComponent)
	{
		SelectedFrameBorder->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	const bool bIsSelected =
		BuildUIComponent->GetSelectedBuildQuickSlotIndex() == SlotIndex;

	SelectedFrameBorder->SetVisibility(
		bIsSelected
			? ESlateVisibility::HitTestInvisible
			: ESlateVisibility::Collapsed
	);
}

void UKOBuildQuickSlotWidget::RefreshSlotNumber()
{
	if (!SlotNumberText)
	{
		return;
	}

	SlotNumberText->SetText(FText::AsNumber(SlotIndex + 1));
	SlotNumberText->SetVisibility(ESlateVisibility::HitTestInvisible);
}
