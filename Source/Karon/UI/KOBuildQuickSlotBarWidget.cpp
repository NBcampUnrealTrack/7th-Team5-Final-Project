#include "KOBuildQuickSlotBarWidget.h"

#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "GameFramework/PlayerController.h"
#include "KOBuildQuickSlotWidget.h"
#include "KOBuildUIComponent.h"

#include "Messaging/KOMessageTypes.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "StructUtils/InstancedStruct.h"

DEFINE_LOG_CATEGORY_STATIC(LogKOBuildSlot, Log, All);

void UKOBuildQuickSlotBarWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	BuildModeChangedCallback.BindDynamic(
		this,
		&UKOBuildQuickSlotBarWidget::HandleBuildModeChangedMessage
	);

	BuildModeChangedHandle = Subscribe(
		KOGameplayTags::Data_Message_Build_ModeChanged,
		BuildModeChangedCallback
	);

	RebuildSlots();
	
	if (UKOGridBuildComponent* GridBuildComponent = GetGridBuildComponent())
	{
		SetBuildModeFrame(GridBuildComponent->GetCurrentMode());
	}
	else
	{
		SetBuildModeFrame(EKOGridBuildMode::None);
	}
}

void UKOBuildQuickSlotBarWidget::NativeDestruct()
{
	Unsubscribe(BuildModeChangedHandle);
	BuildModeChangedHandle = FGameplayMessageHandle();
	BuildModeChangedCallback.Clear();

	Super::NativeDestruct();
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

UKOGridBuildComponent* UKOBuildQuickSlotBarWidget::GetGridBuildComponent() const
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return nullptr;
	}

	return PC->FindComponentByClass<UKOGridBuildComponent>();
}

void UKOBuildQuickSlotBarWidget::SetModeBorderColor(const FLinearColor& InColor)
{
	if (TopModeBorderImage)
	{
		TopModeBorderImage->SetColorAndOpacity(InColor);
	}

	if (BottomModeBorderImage)
	{
		BottomModeBorderImage->SetColorAndOpacity(InColor);
	}

	if (LeftModeBorderImage)
	{
		LeftModeBorderImage->SetColorAndOpacity(InColor);
	}

	if (RightModeBorderImage)
	{
		RightModeBorderImage->SetColorAndOpacity(InColor);
	}
}

void UKOBuildQuickSlotBarWidget::HandleBuildModeChangedMessage(FGameplayTag Channel, const FInstancedStruct& Payload)
{
	const FKOBuildModeChangedMessage* Message =
		Payload.GetPtr<FKOBuildModeChangedMessage>();

	if (!Message)
	{
		return;
	}

	SetBuildModeFrame(Message->NewMode);
}

void UKOBuildQuickSlotBarWidget::SetModeBorderVisible(bool bVisible)
{
	const ESlateVisibility NewVisibility = bVisible
		? ESlateVisibility::HitTestInvisible
		: ESlateVisibility::Collapsed;

	if (TopModeBorderImage)
	{
		TopModeBorderImage->SetVisibility(NewVisibility);
	}

	if (BottomModeBorderImage)
	{
		BottomModeBorderImage->SetVisibility(NewVisibility);
	}

	if (LeftModeBorderImage)
	{
		LeftModeBorderImage->SetVisibility(NewVisibility);
	}

	if (RightModeBorderImage)
	{
		RightModeBorderImage->SetVisibility(NewVisibility);
	}
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

void UKOBuildQuickSlotBarWidget::SetBuildModeFrame(EKOGridBuildMode InMode)
{
	switch (InMode)
	{
	case EKOGridBuildMode::Placing:
		SetModeBorderColor(BuildModeBorderColor);
		SetModeBorderVisible(true);
		break;

	case EKOGridBuildMode::Destroying:
		SetModeBorderColor(DestroyModeBorderColor);
		SetModeBorderVisible(true);
		break;

	case EKOGridBuildMode::BuildMenu:
	case EKOGridBuildMode::None:
	default:
		SetModeBorderVisible(false);
		break;
	}
}


