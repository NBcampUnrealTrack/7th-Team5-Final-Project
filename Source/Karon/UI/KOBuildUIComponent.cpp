#include "KOBuildUIComponent.h"

#include "Blueprint/UserWidget.h"
#include "CommonActivatableWidget.h"
#include "GameFramework/PlayerController.h"
#include "Component/KOGridBuildComponent.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "KOUISubsystem.h"

#include "Messaging/KOMessageTypes.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "StructUtils/InstancedStruct.h"

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

void UKOBuildUIComponent::OpenQuickSlotBar()
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return;
	}

	if (!QuickSlotBarWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BuildUI] QuickSlotBarWidgetClass가 설정되지 않았습니다."));
		return;
	}

	if (IsValid(QuickSlotBarWidget))
	{
		return;
	}

	UKOUISubsystem* UISubsystem = UKOUISubsystem::Get(PC);
	if (!UISubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BuildUI] KOUISubsystem을 찾을 수 없습니다."));
		return;
	}

	QuickSlotBarWidget = UISubsystem->PushLayer(
		KOGameplayTags::UI_Layer_Game,
		QuickSlotBarWidgetClass
	);

	if (!QuickSlotBarWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BuildUI] 퀵슬롯 바 Push 실패"));
	}
}

void UKOBuildUIComponent::CloseQuickSlotBar()
{
	APlayerController* PC = GetOwningPlayerController();

	if (!QuickSlotBarWidget)
	{
		return;
	}

	if (PC)
	{
		if (UKOUISubsystem* UISubsystem = UKOUISubsystem::Get(PC))
		{
			UISubsystem->PopLayer(QuickSlotBarWidget);
		}
	}

	QuickSlotBarWidget = nullptr;
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
	
	if (IsBuildAssignMenuOpen())
	{
		CloseBuildAssignMenu();
	}

	GridBuildComponent->EnterBuildMenuMode();
	
	// 건설 모드에서는 퀵슬롯만 보여준다.
	OpenQuickSlotBar();

	PC->bShowMouseCursor = false;
	
	FInputModeGameOnly InputMode;
	PC->SetInputMode(InputMode);
	
	UE_LOG(LogTemp, Log, TEXT("[BuildUI] 건설 모드 열림"));
}

void UKOBuildUIComponent::CloseBuildMenu()
{
	CloseBuildAssignMenu();
	
	CloseQuickSlotBar();
	
	if (UKOGridBuildComponent* GridBuildComponent = GetGridBuildComponent())
	{
		GridBuildComponent->ExitBuildMenuMode();
	}
	
	APlayerController* PC = GetOwningPlayerController();

	if (PC)
	{
		PC->bShowMouseCursor = false;

		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);

		PC->SetIgnoreMoveInput(false); // 이동 가능
		PC->SetIgnoreLookInput(false); // 카메라 회전 가능
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

void UKOBuildUIComponent::OpenBuildAssignMenu()
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return;
	}

	if (IsBuildAssignMenuOpen())
	{
		return;
	}
	
	UKOGridBuildComponent* GridBuildComponent = GetGridBuildComponent();
	
	const bool bWasBuildMenuOpen = IsBuildMenuOpen();
	
	// 건설 모드 중 I키를 누르면 기존 동작 유지:
	// 설치 모드/파괴 모드에서 BuildMenu 상태로 복귀한 뒤 설비 선택 창을 연다.
	if (bWasBuildMenuOpen && GridBuildComponent)
	{
		if (GridBuildComponent->IsBuildMode())
		{
			GridBuildComponent->CancelBuildMode();
		}

		if (GridBuildComponent->IsDestroyMode())
		{
			GridBuildComponent->CancelDestroyMode();
		}
	}

	// 건설 모드 밖에서 I를 눌러도 퀵슬롯은 보여야 한다.
	OpenQuickSlotBar();
	
	if (!BuildAssignMenuWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BuildUI] BuildAssignMenuWidgetClass가 설정되지 않았습니다."));
		return;
	}
	
	UKOUISubsystem* UISubsystem = UKOUISubsystem::Get(PC);
	if (!UISubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BuildUI] KOUISubsystem을 찾을 수 없습니다."));
		return;
	}
	
	BuildAssignMenuWidget = UISubsystem->PushLayer(
		KOGameplayTags::UI_Layer_GameMenu,
		BuildAssignMenuWidgetClass
	);

	if (!BuildAssignMenuWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BuildUI] 설비 할당 UI Push 실패"));
		return;
	}
	
	PC->bShowMouseCursor = true;

	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(BuildAssignMenuWidget->TakeWidget());
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PC->SetInputMode(InputMode);
	
	PC->SetIgnoreMoveInput(true); // 이동 불가능
	PC->SetIgnoreLookInput(true); // 키메라 회전 불가능

	UE_LOG(LogTemp, Log, TEXT("[BuildUI] 설비 할당 UI 열림"));
}

void UKOBuildUIComponent::CloseBuildAssignMenu()
{
	APlayerController* PC = GetOwningPlayerController();
	
	if (PC)
	{
		if (UKOUISubsystem* UISubsystem = UKOUISubsystem::Get(PC))
		{
			if (BuildAssignMenuWidget)
			{
				UISubsystem->PopLayer(BuildAssignMenuWidget);
				BuildAssignMenuWidget = nullptr;
			}
		}
		
		PC->bShowMouseCursor = false;

		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		
		PC->SetIgnoreMoveInput(false); // 이동 가능
		PC->SetIgnoreLookInput(false); // 카메라 회전 가능
	}
	else
	{
		BuildAssignMenuWidget = nullptr;
	}

	// 건설 모드가 아닌 상태에서 I로 설비 선택 창을 연 경우,
	// 창을 닫으면 퀵슬롯도 같이 닫는다.
	if (!IsBuildMenuOpen())
	{
		CloseQuickSlotBar();
	}
	
	UE_LOG(LogKOBuildUI, Log, TEXT("[BuildUI] 설비 할당 UI 닫힘"));
}

void UKOBuildUIComponent::ToggleBuildAssignMenu()
{
	if (IsBuildAssignMenuOpen())
	{
		CloseBuildAssignMenu();
		return;
	}
	OpenBuildAssignMenu();
}

bool UKOBuildUIComponent::IsBuildAssignMenuOpen() const
{
	return IsValid(BuildAssignMenuWidget);
}

void UKOBuildUIComponent::EscapeBuildAction()
{
	if (IsBuildAssignMenuOpen())
	{
		CloseBuildAssignMenu();
		return;
	}

	UKOGridBuildComponent* GridBuildComponent = GetGridBuildComponent();
	if (!GridBuildComponent)
	{
		return;
	}

	if (GridBuildComponent->IsBuildMode())
	{
		GridBuildComponent->CancelBuildMode();
		return;
	}

	if (GridBuildComponent->IsDestroyMode())
	{
		GridBuildComponent->CancelDestroyMode();
		return;
	}

	if (GridBuildComponent->IsBuildMenuMode())
	{
		// ESC로도 건설 모드 종료 금지
		return;
	}
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

void UKOBuildUIComponent::SelectBuildQuickSlot(int32 SlotIndex)
{
	if (IsBuildAssignMenuOpen())
	{
		return;
	}
	
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
		UE_LOG(LogKOBuildUI, Warning, TEXT("[BuildUI] 퀵슬롯 %d가 비어 있습니다."), SlotIndex + 1);
		return;
	}

	UKOGridBuildComponent* GridBuildComponent = GetGridBuildComponent();
	if (!GridBuildComponent)
	{
		return;
	}

	GridBuildComponent->StartBuildModeWithId(FactoryId);
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

void UKOBuildUIComponent::StartDestroyBuildMode()
{
	if (IsBuildAssignMenuOpen())
	{
		return;
	}
	
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
}

void UKOBuildUIComponent::ConfirmBuildAction()
{
	if (IsBuildAssignMenuOpen())
	{
		return;
	}
	
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
	if (IsBuildAssignMenuOpen())
	{
		return;
	}
	
	UKOGridBuildComponent* GridBuildComponent = GetGridBuildComponent();
	if (!GridBuildComponent)
	{
		return;
	}

	if (GridBuildComponent->IsBuildMode())
	{
		GridBuildComponent->CancelBuildMode();
		return;
	}
	
	if (GridBuildComponent->IsDestroyMode())
	{
		GridBuildComponent->CancelDestroyMode();
		return;
	}
	
	if (GridBuildComponent->IsBuildMenuMode())
	{
		// 우클릭으로는 건설 모드 종료 금지
		return;
	}
}