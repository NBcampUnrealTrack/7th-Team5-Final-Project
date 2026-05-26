#include "KOBuildQuickSlotWidget.h"

#include "Components/Image.h"
#include "GameFramework/PlayerController.h"
#include "KOBuildDragDropOperation.h"
#include "KOBuildUIComponent.h"

#include "Messaging/KOMessageTypes.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "StructUtils/InstancedStruct.h"
#include "Subsystem/KOLoadSubsystem.h"

void UKOBuildQuickSlotWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	RefreshSlot();
}

void UKOBuildQuickSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	Callback.BindDynamic(
		this,
		&UKOBuildQuickSlotWidget::HandleQuickSlotChangedMessage
	);

	QuickSlotChangedHandle = Subscribe(
		KOGameplayTags::Data_Message_Build_QuickSlotChanged,
		Callback
	);

	RefreshSlot();
}

void UKOBuildQuickSlotWidget::NativeDestruct()
{
	Unsubscribe(QuickSlotChangedHandle);
	QuickSlotChangedHandle = FGameplayMessageHandle();

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

void UKOBuildQuickSlotWidget::SetupSlot(int32 InSlotIndex)
{
	SlotIndex = InSlotIndex;

	RefreshSlot();
}

void UKOBuildQuickSlotWidget::RefreshSlot()
{
	if (!SlotIconImage)
	{
		return;
	}

	UKOBuildUIComponent* BuildUIComponent = GetBuildUIComponent();
	if (!BuildUIComponent)
	{
		if (EmptySlotIcon)
		{
			SlotIconImage->SetBrushFromTexture(EmptySlotIcon);
		}
		return;
	}

	const FName AssignedFactoryId = BuildUIComponent->GetBuildQuickSlot(SlotIndex);

	if (AssignedFactoryId.IsNone())
	{
		if (EmptySlotIcon)
		{
			SlotIconImage->SetBrushFromTexture(EmptySlotIcon);
		}

		return;
	}
	
	UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this);
	if (!LoadSub)
	{
		if (EmptySlotIcon)
		{
			SlotIconImage->SetBrushFromTexture(EmptySlotIcon);
		}

		return;
	}

	UTexture2D* Icon = LoadSub->ResolveFactoryIcon(AssignedFactoryId);
	if (!Icon)
	{
		if (EmptySlotIcon)
		{
			SlotIconImage->SetBrushFromTexture(EmptySlotIcon);
		}

		return;
	}

	SlotIconImage->SetBrushFromTexture(Icon);
}

bool UKOBuildQuickSlotWidget::NativeOnDrop(
	const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation
)
{
	UKOBuildDragDropOperation* BuildDragOperation = Cast<UKOBuildDragDropOperation>(InOperation);

	if (!BuildDragOperation)
	{
		return false;
	}

	if (BuildDragOperation->FactoryId.IsNone())
	{
		return false;
	}

	UKOBuildUIComponent* BuildUIComponent = GetBuildUIComponent();
	if (!BuildUIComponent)
	{
		return false;
	}

	BuildUIComponent->SetBuildQuickSlot(SlotIndex, BuildDragOperation->FactoryId);

	return BuildUIComponent->SetBuildQuickSlot(SlotIndex, BuildDragOperation->FactoryId);
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