#include "KOBuildQuickSlotBarWidget.h"

#include "Components/PanelWidget.h"
#include "GameFramework/PlayerController.h"
#include "KOBuildQuickSlotWidget.h"
#include "KOBuildUIComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogKOBuildSlot, Log, All);

void UKOBuildQuickSlotBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	RebuildSlots();
}

UKOBuildUIComponent* UKOBuildQuickSlotBarWidget::GetBuildUIComponent() const
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return nullptr;
	}

	return PC->FindComponentByClass<UKOBuildUIComponent>();
}

void UKOBuildQuickSlotBarWidget::RebuildSlots()
{
	if (!SlotContainer)
	{
		UE_LOG(LogKOBuildSlot, Warning, TEXT("[BuildQuickSlotBar] SlotContainer가 없습니다."));
		return;
	}

	if (!QuickSlotWidgetClass)
	{
		UE_LOG(LogKOBuildSlot, Warning, TEXT("[BuildQuickSlotBar] QuickSlotWidgetClass가 설정되지 않았습니다."));
		return;
	}

	UKOBuildUIComponent* BuildUIComponent = GetBuildUIComponent();
	if (!BuildUIComponent)
	{
		UE_LOG(LogKOBuildSlot, Warning, TEXT("[BuildQuickSlotBar] KOBuildUIComponent를 찾을 수 없습니다."));
		return;
	}

	SlotContainer->ClearChildren();

	const int32 SlotCount = BuildUIComponent->GetQuickSlotCount();

	for (int32 SlotIndex = 0; SlotIndex < SlotCount; ++SlotIndex)
	{
		UKOBuildQuickSlotWidget* SlotWidget =
			CreateWidget<UKOBuildQuickSlotWidget>(
				GetOwningPlayer(),
				QuickSlotWidgetClass
			);

		if (!SlotWidget)
		{
			continue;
		}

		SlotWidget->SetupSlot(SlotIndex);

		SlotContainer->AddChild(SlotWidget);
	}
}