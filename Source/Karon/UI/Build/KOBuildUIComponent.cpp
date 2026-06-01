#include "UI/Build/KOBuildUIComponent.h"

#include "Blueprint/UserWidget.h"
#include "CommonActivatableWidget.h"
#include "GameFramework/PlayerController.h"
#include "Component/Build/KOGridBuildComponent.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "UI/KOUISubsystem.h"

#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Component/Inventory/KOInventoryComponent.h"
#include "StructUtils/InstancedStruct.h"
#include "Utility/Messaging/KOMessageTypes.h"

DEFINE_LOG_CATEGORY_STATIC(LogKOBuildUI, Log, All);

UKOBuildUIComponent::UKOBuildUIComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UKOBuildUIComponent::BeginPlay()
{
	Super::BeginPlay();

	QuickSlotCount = FMath::Max(1, QuickSlotCount);
	BuildQuickSlots.SetNum(QuickSlotCount);
}

APlayerController* UKOBuildUIComponent::GetOwningPlayerController() const
{
	return Cast<APlayerController>(GetOwner());
}

UKOGridBuildComponent* UKOBuildUIComponent::GetGridBuildComponent() const
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return nullptr;
	}

	return OwnerActor->FindComponentByClass<UKOGridBuildComponent>();
}

UKOInventoryComponent* UKOBuildUIComponent::GetInventoryComponent() const
{
	APlayerController* PC = GetOwningPlayerController();
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

void UKOBuildUIComponent::SetSelectedBuildQuickSlot(int32 NewSlotIndex)
{
	if (SelectedQuickSlotIndex == NewSlotIndex)
	{
		return;
	}

	const int32 PreviousSlotIndex = SelectedQuickSlotIndex;
	SelectedQuickSlotIndex = NewSlotIndex;

	FKOBuildQuickSlotSelectionChangedMessage Message;
	Message.PreviousSlotIndex = PreviousSlotIndex;
	Message.NewSlotIndex = SelectedQuickSlotIndex;

	Broadcast(
		KOGameplayTags::Data_Message_Build_QuickSlotSelectionChanged,
		FInstancedStruct::Make(Message)
	);

	UE_LOG(
		LogKOBuildUI,
		Log,
		TEXT("[BuildUI] 선택 퀵슬롯 변경: %d -> %d"),
		PreviousSlotIndex + 1,
		SelectedQuickSlotIndex + 1
	);
}

void UKOBuildUIComponent::ClearSelectedBuildQuickSlot()
{
	SetSelectedBuildQuickSlot(INDEX_NONE);
}

void UKOBuildUIComponent::OpenBuildMenu()
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		UE_LOG(LogKOBuildUI, Warning, TEXT("[BuildUI] Owner가 PlayerController가 아닙니다."));
		return;
	}

	UKOGridBuildComponent* GridBuildComponent = GetGridBuildComponent();
	if (!GridBuildComponent)
	{
		UE_LOG(LogKOBuildUI, Warning, TEXT("[BuildUI] KOGridBuildComponent를 찾을 수 없습니다."));
		return;
	}

	GridBuildComponent->EnterBuildMenuMode();
	
	OpenQuickSlotBar();

	// 퀵슬롯 BP가 InputMode = All이어도 건설 모드에서는 마우스를 숨긴다.
	PC->bShowMouseCursor = false;
}

void UKOBuildUIComponent::CloseBuildMenu()
{	
	ClearSelectedBuildQuickSlot();
	CloseQuickSlotBar();
	
	if (UKOGridBuildComponent* GridBuildComponent = GetGridBuildComponent())
	{
		GridBuildComponent->ExitBuildMenuMode();
	}
	
	UE_LOG(LogKOBuildUI, Log, TEXT("[BuildUI] 건설 모드 종료"));
}

void UKOBuildUIComponent::ToggleBuildMenu()
{
	if (IsBuildMenuOpen())
	{
		CloseBuildMenu();
	}
	else
	{
		OpenBuildMenu();
	}
}

bool UKOBuildUIComponent::IsBuildMenuOpen() const
{
	const UKOGridBuildComponent* GridBuildComponent = GetGridBuildComponent();
	return GridBuildComponent && GridBuildComponent->IsBuildSystemActive();
}

bool UKOBuildUIComponent::SetBuildQuickSlot(int32 SlotIndex, FName FactoryId)
{
	if (!BuildQuickSlots.IsValidIndex(SlotIndex))
	{
		UE_LOG(LogKOBuildUI, Warning, TEXT("[BuildUI] 잘못된 퀵슬롯 인덱스: %d"), SlotIndex);
		return false;
	}

	if (FactoryId.IsNone())
	{
		UE_LOG(LogKOBuildUI, Warning, TEXT("[BuildUI] FactoryId가 비어 있습니다."));
		return false;
	}
	
	const UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this);
	if (!LoadSub || !LoadSub->FindFactoryRow(FactoryId))
	{
		UE_LOG(LogKOBuildUI, Warning, TEXT("[BuildUI] 존재하지 않는 FactoryId입니다: %s"),
			*FactoryId.ToString()
		);
		return false;
	}

	BuildQuickSlots[SlotIndex] = FactoryId;
	
	FKOBuildQuickSlotChangedMessage Message;
	Message.SlotIndex = SlotIndex;
	Message.FactoryId = FactoryId;
	
	Broadcast(KOGameplayTags::Data_Message_Build_QuickSlotChanged, FInstancedStruct::Make(Message));

	UE_LOG(LogKOBuildUI, Log, TEXT("[BuildUI] 퀵슬롯 %d 등록: %s"),
		SlotIndex + 1,
		*FactoryId.ToString()
	);
	
	return true;
}

bool UKOBuildUIComponent::SwapBuildQuickSlot(int32 SlotIndexA, int32 SlotIndexB)
{
	if (SlotIndexA == SlotIndexB)
	{
		return false;
	}
	if (!BuildQuickSlots.IsValidIndex(SlotIndexA) || !BuildQuickSlots.IsValidIndex(SlotIndexB))
	{
		UE_LOG(LogKOBuildUI, Warning, TEXT("[BuildUI] 잘못된 퀵슬롯 인덱스 스왑: %d <-> %d"), SlotIndexA, SlotIndexB);
		return false;
	}

	BuildQuickSlots.Swap(SlotIndexA, SlotIndexB);

	auto BroadcastSlot = [this](int32 SlotIndex)
	{
		FKOBuildQuickSlotChangedMessage Message;
		Message.SlotIndex = SlotIndex;
		Message.FactoryId = BuildQuickSlots[SlotIndex];
		Broadcast(KOGameplayTags::Data_Message_Build_QuickSlotChanged, FInstancedStruct::Make(Message));
	};

	BroadcastSlot(SlotIndexA);
	BroadcastSlot(SlotIndexB);

	UE_LOG(LogKOBuildUI, Log, TEXT("[BuildUI] 퀵슬롯 스왑: %d <-> %d"), SlotIndexA + 1, SlotIndexB + 1);
	return true;
}

void UKOBuildUIComponent::SelectBuildQuickSlot(int32 SlotIndex)
{	
	if (!IsBuildMenuOpen())
	{
		UE_LOG(LogKOBuildUI, Warning, TEXT("[BuildUI] 건설 메뉴가 열려 있지 않습니다."));
		return;
	}

	if (!BuildQuickSlots.IsValidIndex(SlotIndex))
	{
		return;
	}

	const FName FactoryId = BuildQuickSlots[SlotIndex];

	if (FactoryId.IsNone())
	{
		UE_LOG(LogKOBuildUI, Warning,
			TEXT("[BuildUI] 퀵슬롯 %d가 비어 있습니다. (This=%p, Owner=%s, SlotCount=%d)"),
			SlotIndex + 1,
			this,
			*GetNameSafe(GetOwner()),
			BuildQuickSlots.Num()
		);

		for (int32 Idx = 0; Idx < BuildQuickSlots.Num(); ++Idx)
		{
			UE_LOG(LogKOBuildUI, Warning, TEXT("[BuildUI]   슬롯[%d] = %s"),
				Idx,
				*BuildQuickSlots[Idx].ToString()
			);
		}
		return;
	}
	
	UKOInventoryComponent* InventoryComponent = GetInventoryComponent();
	if (!InventoryComponent || !InventoryComponent->HasEnoughItems(FactoryId, 1))
	{
		UE_LOG(LogKOBuildUI, Warning, TEXT("[BuildUI] 퀵슬롯 %d 설비 수량이 없습니다: %s"),
			SlotIndex + 1,
			*FactoryId.ToString()
		);
		return;
	}

	UKOGridBuildComponent* GridBuildComponent = GetGridBuildComponent();
	if (!GridBuildComponent)
	{
		return;
	}

	GridBuildComponent->StartBuildModeWithId(FactoryId);
	
	SetSelectedBuildQuickSlot(SlotIndex);
}

FName UKOBuildUIComponent::GetBuildQuickSlot(int32 SlotIndex) const
{
	if (!BuildQuickSlots.IsValidIndex(SlotIndex))
	{
		return NAME_None;
	}

	return BuildQuickSlots[SlotIndex];
}

int32 UKOBuildUIComponent::GetQuickSlotCount() const
{
	return BuildQuickSlots.Num();
}

int32 UKOBuildUIComponent::GetSelectedBuildQuickSlotIndex() const
{
	return SelectedQuickSlotIndex;
}

void UKOBuildUIComponent::OpenQuickSlotBar()
{
	// 호출 일원화: 서브시스템을 직접 잡지 않고 GMS 경로로 열기 요청.
	UKOUISubsystem::RequestOpenWidget(this, KOGameplayTags::UI_Widget_QuickSlotBar);
}

void UKOBuildUIComponent::CloseQuickSlotBar()
{
	UKOUISubsystem::RequestCloseWidget(this, KOGameplayTags::UI_Widget_QuickSlotBar);
}

void UKOBuildUIComponent::StartDestroyBuildMode()
{
	if (!IsBuildMenuOpen())
	{
		UE_LOG(LogKOBuildUI, Warning, TEXT("[BuildUI] 건설 메뉴가 열려 있지 않아 파괴 모드로 들어갈 수 없습니다."));
		return;
	}

	UKOGridBuildComponent* GridBuildComponent = GetGridBuildComponent();
	if (!GridBuildComponent)
	{
		return;
	}

	GridBuildComponent->StartDestroyMode();
	
	ClearSelectedBuildQuickSlot();
}

void UKOBuildUIComponent::ToggleDestroyBuildMode()
{
	if (!IsBuildMenuOpen())
	{
		UE_LOG(LogKOBuildUI, Warning, TEXT("[BuildUI] 건설 메뉴가 열려 있지 않아 파괴 모드 토글 불가."));
		return;
	}

	UKOGridBuildComponent* GridBuildComponent = GetGridBuildComponent();
	if (!GridBuildComponent)
	{
		return;
	}

	if (GridBuildComponent->IsDestroyMode())
	{
		GridBuildComponent->CancelDestroyMode();
	}
	else
	{
		GridBuildComponent->StartDestroyMode();
		ClearSelectedBuildQuickSlot();
	}
}

void UKOBuildUIComponent::ConfirmBuildAction()
{
	UKOGridBuildComponent* GridBuildComponent = GetGridBuildComponent();
	if (!GridBuildComponent)
	{
		return;
	}

	if (GridBuildComponent->IsBuildMode())
	{
		GridBuildComponent->RequestBuild();
		return;
	}
	
	if (GridBuildComponent->IsDestroyMode())
	{
		GridBuildComponent->RequestDestroy();
		return;
	}
}

void UKOBuildUIComponent::CancelBuildAction()
{	
	UKOGridBuildComponent* GridBuildComponent = GetGridBuildComponent();
	if (!GridBuildComponent)
	{
		return;
	}

	if (GridBuildComponent->IsBuildMode())
	{
		GridBuildComponent->CancelBuildMode();
		ClearSelectedBuildQuickSlot();
		return;
	}
	
	if (GridBuildComponent->IsDestroyMode())
	{
		GridBuildComponent->CancelDestroyMode();
		ClearSelectedBuildQuickSlot();
		return;
	}
	
	if (GridBuildComponent->IsBuildMenuMode())
	{
		// 우클릭으로는 건설 모드 종료 금지
		return;
	}
}
