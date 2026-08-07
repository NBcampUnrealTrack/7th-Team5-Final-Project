// Copyright Karon Team 5. All Rights Reserved.

#include "KOTeleportWidget.h"
#include "KOBonfireEntryObject.h"
#include "Subsystem/KOTeleportSubsystem.h"
#include "UI/KOUISubsystem.h"
#include "UI/Loading/KOLoadingUiSubsystem.h"
#include "AbilitySystem/Tag/UI/KOGameplayTags_UI.h"

#include "CommonButtonBase.h"
#include "CommonListView.h"

void UKOTeleportWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	CachedTeleportSubsystem  = GetOwningLocalPlayer()->GetSubsystem<UKOTeleportSubsystem>();
	CachedUISubsystem		 = GetOwningLocalPlayer()->GetSubsystem<UKOUISubsystem>();
	CachedLoadingUISubsystem = GetOwningLocalPlayer()->GetGameInstance()->GetSubsystem<UKOLoadingUiSubsystem>();
}

void UKOTeleportWidget::NativeDestruct()
{
	CachedTeleportSubsystem  = nullptr;
	CachedUISubsystem		 = nullptr;
	CachedLoadingUISubsystem = nullptr;
	
	Super::NativeDestruct();
}

void UKOTeleportWidget::NativeOnActivated()
{
	Super::NativeOnActivated();
	
	BindEvents();
	
	RefreshList();
}

void UKOTeleportWidget::NativeOnDeactivated()
{
	RemoveEvents();
	
	Super::NativeOnDeactivated();
}

void UKOTeleportWidget::RefreshList()
{
	if (BonfireListView)
	{
		BonfireListView->ClearListItems();
	
		const TArray<FBonfireUIData> Bonfires = CachedTeleportSubsystem->GetActivatedBonfires();
	
		for (const FBonfireUIData& Data : Bonfires)
		{
			UKOBonfireEntryObject* Entry = NewObject<UKOBonfireEntryObject>(this);
		
			Entry->Initialize(Data);
		
			BonfireListView->AddItem(Entry);
			
			UE_LOG(LogTemp, Warning, TEXT("%s"), *Data.DisplayName.ToString());
		}
		
		if (BonfireListView->GetNumItems() > 0)
		{
			BonfireListView->SetSelectedIndex(0);
		}
	}
}

void UKOTeleportWidget::BindEvents()
{
	if (BonfireListView)
	{
		BonfireListView->OnItemSelectionChanged().AddUObject(
			this, &ThisClass::HandleSelectionChanged);
	}
	
	if (MoveButton)
	{
		MoveButton->OnClicked().AddUObject(
			this, &ThisClass::HandleMoveClicked);
	}
	
	if (CancelButton)
	{
		CancelButton->OnClicked().AddUObject(
			this, &ThisClass::HandleCancelClicked);
	}
}

void UKOTeleportWidget::RemoveEvents()
{
	if (BonfireListView->OnItemSelectionChanged().IsBound())
	{
		BonfireListView->OnItemSelectionChanged().RemoveAll(this);
	}
	
	if (MoveButton->OnClicked().IsBound())
	{
		MoveButton->OnClicked().RemoveAll(this);
	}
	
	if (CancelButton->OnClicked().IsBound())
	{
		CancelButton->OnClicked().RemoveAll(this);
	}
}

void UKOTeleportWidget::HandleSelectionChanged(UObject* SelectedItem)
{
	CurrentSelectedEntry = Cast<UKOBonfireEntryObject>(SelectedItem);
	
	if (CurrentSelectedEntry == nullptr)
	{
		return;
	}
}

void UKOTeleportWidget::HandleMoveClicked()
{
	if (CurrentSelectedEntry == nullptr)
	{
		return;
	}
	
	if (CachedUISubsystem)
	{
		CachedUISubsystem->CloseWidget(GetWorld(), KOGameplayTags::UI_Widget_TeleportPopup);
	}
	
	if (CurrentSelectedEntry->GetBonfireID() == CachedTeleportSubsystem->GetRequestActorName())
	{
		CachedTeleportSubsystem->Teleport(CurrentSelectedEntry->GetBonfireID());
	}
	else
	{
		CachedLoadingUISubsystem->ShowLoadingScreen(LoadingWidget);
		CachedTeleportSubsystem->Teleport(CurrentSelectedEntry->GetBonfireID());
		CachedLoadingUISubsystem->HideLoadingScreen();
	}
}

void UKOTeleportWidget::HandleCancelClicked()
{
	if (CachedUISubsystem)
	{
		CachedUISubsystem->CloseWidget(GetWorld(), KOGameplayTags::UI_Widget_TeleportPopup);
	}
}
