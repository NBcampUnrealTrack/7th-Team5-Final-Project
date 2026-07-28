// Copyright Karon Team 5. All Rights Reserved.

#include "KOTeleportWidget.h"

#include "CommonListView.h"
#include "Subsystem/KOTeleportSubsystem.h"

void UKOTeleportWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	CachedTeleportSubsystem = ULocalPlayer::GetSubsystem<UKOTeleportSubsystem>();
}

void UKOTeleportWidget::NativeDestruct()
{
	CachedTeleportSubsystem = nullptr;
	
	Super::NativeDestruct();
}

void UKOTeleportWidget::NativeOnActivated()
{
	Super::NativeOnActivated();
	
	RefreshList();
}

void UKOTeleportWidget::RefreshList()
{
	BonfireListView->ClearListItems();
	
	const TArray<FBonfireUIData> Bonfires = CachedTeleportSubsystem->GetActivatedBonfires();
	
	for (const FBonfireUIData& Data : Bonfires)
	{
		//
	}
}
