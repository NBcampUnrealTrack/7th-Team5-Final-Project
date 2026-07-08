#include "KOBuildUIComponent.h"

#include "GameFramework/PlayerController.h"
#include "Component/Build/KOGridBuildComponent.h"
#include "Component/Inventory/KOInventoryComponent.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "UI/KOUISubsystem.h"

#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "StructUtils/InstancedStruct.h"
#include "Subsystem/KOQuestGuideSubsystem.h"
#include "UI/HUD/KOInGameHUD.h"
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
	
	SetHUDKeyGuideMode(true);
	
	OpenQuickSlotBar();
}

void UKOBuildUIComponent::CloseBuildMenu()
{	
	ClearSelectedBuildQuickSlot();
	CloseQuickSlotBar();
	
	if (UKOGridBuildComponent* GridBuildComponent = GetGridBuildComponent())
	{
		GridBuildComponent->ExitBuildMenuMode();
	}
	
	SetHUDKeyGuideMode(false);
	
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

	auto BroadcastSlot = [this](int32 ChangedSlotIndex)
	{
		FKOBuildQuickSlotChangedMessage Message;
		Message.SlotIndex = ChangedSlotIndex;
		Message.FactoryId = BuildQuickSlots[ChangedSlotIndex];

		Broadcast(KOGameplayTags::Data_Message_Build_QuickSlotChanged, FInstancedStruct::Make(Message));
	};

	bool bMovedSelectedSlot = false;

	// 같은 FactoryId가 이미 다른 퀵슬롯에 있으면 기존 슬롯을 비운다.
	for (int32 Index = 0; Index < BuildQuickSlots.Num(); ++Index)
	{
		if (Index == SlotIndex)
		{
			continue;
		}

		if (BuildQuickSlots[Index] != FactoryId)
		{
			continue;
		}

		BuildQuickSlots[Index] = NAME_None;
		BroadcastSlot(Index);

		// 기존 선택 중이돈 슬롯이 비워진 경우
		if (SelectedQuickSlotIndex == Index)
		{
			bMovedSelectedSlot = true;
		}

		UE_LOG(LogKOBuildUI, Log, TEXT("[BuildUI] 중복 퀵슬롯 해제: 슬롯 %d / %s"),
			Index + 1,
			*FactoryId.ToString()
		);
	}

	// 새 슬롯에 할당
	BuildQuickSlots[SlotIndex] = FactoryId;
	BroadcastSlot(SlotIndex);

	// 기존에 선택된 슬롯이 해제된 경우, 선택 상태를 새 슬롯으로 옮긴다.
	if (bMovedSelectedSlot)
	{
		SetSelectedBuildQuickSlot(SlotIndex);
	}
	
	// 퀘스트
	if (UKOQuestGuideSubsystem* QuestGuide = UKOQuestGuideSubsystem::Get(this))
	{
		QuestGuide->NotifyBuildQuickSlotAssigned(FactoryId);
	}

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
	
	UKOGridBuildComponent* GridBuildComponent = GetGridBuildComponent();
	if (!GridBuildComponent)
	{
		return;
	}

	const FName FactoryId = BuildQuickSlots[SlotIndex];

	// 빈 슬롯도 선택 가능. 배치 중이던 고스트 프리뷰, 파괴 타겟 프리뷰 제거	
	UKOInventoryComponent* InventoryComponent = GetInventoryComponent();
	if (FactoryId.IsNone() || !InventoryComponent || !InventoryComponent->HasEnoughItems(FactoryId, 1))
	{
		if (GridBuildComponent->IsBuildMode())
		{
			GridBuildComponent->CancelBuildMode();
		}
		else if (GridBuildComponent->IsDestroyMode())
		{
			GridBuildComponent->CancelDestroyMode();
		}

		SetSelectedBuildQuickSlot(SlotIndex);
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

void UKOBuildUIComponent::LoadBuildQuickSlotsFromSave(const TArray<FName>& InBuildQuickSlots)
{
	QuickSlotCount = FMath::Max(1, QuickSlotCount);
    BuildQuickSlots.SetNum(QuickSlotCount);
    
    const UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this);
    
    	for (int32 Index = 0; Index < BuildQuickSlots.Num(); ++Index)
    	{
    		FName LoadedFactoryId = NAME_None;
    
    		if (InBuildQuickSlots.IsValidIndex(Index))
    		{
    			LoadedFactoryId = InBuildQuickSlots[Index];
    		}
    
    		if (!LoadedFactoryId.IsNone())
    		{
    			if (!LoadSub || !LoadSub->FindFactoryRow(LoadedFactoryId))
    			{
    				UE_LOG(
    					LogKOBuildUI,
    					Warning,
    					TEXT("[BuildUI] 저장된 퀵슬롯 FactoryId가 유효하지 않습니다. Slot=%d, FactoryId=%s"),
    					Index + 1,
    					*LoadedFactoryId.ToString()
    				);
    
    				LoadedFactoryId = NAME_None;
    			}
    		}
    
    		BuildQuickSlots[Index] = LoadedFactoryId;
    
    		FKOBuildQuickSlotChangedMessage Message;
    		Message.SlotIndex = Index;
    		Message.FactoryId = BuildQuickSlots[Index];
    
    		Broadcast(
    			KOGameplayTags::Data_Message_Build_QuickSlotChanged,
    			FInstancedStruct::Make(Message)
    		);
    	}
    
    	ClearSelectedBuildQuickSlot();
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
	UKOUISubsystem* UISubsystem = UKOUISubsystem::Get(this);
	if (!UISubsystem)
	{
		return;
	}
	
	UKOUISubsystem::OpenWidget(this, KOGameplayTags::UI_Widget_QuickSlotBar);
}

void UKOBuildUIComponent::CloseQuickSlotBar()
{
	UKOUISubsystem::CloseWidget(this, KOGameplayTags::UI_Widget_QuickSlotBar);
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

void UKOBuildUIComponent::RotateBuildPreview(int32 Direction)
{
	UKOGridBuildComponent* GridBuildComponent = GetGridBuildComponent();
	if (!GridBuildComponent)
	{
		return;
	}

	if (!GridBuildComponent->IsBuildMode())
	{
		return;
	}

	GridBuildComponent->RotatePlacementPreview(Direction);
}

void UKOBuildUIComponent::SetHUDKeyGuideMode(bool bBuildMode)
{
	UKOInGameHUD* HUD = GetHUDWidget();

	if (!HUD)
	{
		UE_LOG(LogKOBuildUI, Warning, TEXT("[BuildUI] InGameHUD를 찾지 못했습니다."));
		return;
	}

	HUD->SetBuildKeyGuideMode(bBuildMode);
}

UKOInGameHUD* UKOBuildUIComponent::GetHUDWidget() const
{
	UKOUISubsystem* UISubsystem = UKOUISubsystem::Get(this);
	if (!UISubsystem)
	{
		return nullptr;
	}

	UCommonActivatableWidget* ActiveWidget =
		UISubsystem->FindActiveWidget(KOGameplayTags::UI_Widget_InGameHUD);

	return Cast<UKOInGameHUD>(ActiveWidget);
}
