// Fill out your copyright notice in the Description page of Project Settings.


#include "KOItemDropActor.h"

#include "AbilitySystem/Tag/Event/KOGameplayTags_Event.h"
#include "Items/KOItemLibrary.h"
#include "Items/KOItemSlot.h"
#include "Subsystem/KOSaveSubsystem.h"
#include "Utility/Messaging/KOMessageTypes.h"


// Sets default values
AKOItemDropActor::AKOItemDropActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

bool AKOItemDropActor::CanInteract(AActor* Interactor) const
{
	return bHasItem;
}

void AKOItemDropActor::OnInteract(AActor* Interactor)
{
	if (!bHasItem)
	{
		return;
	}

	bHasItem = false;
	MarkCollectedForSave();
	
	for (const FEnemyDropItemInfo& DropItem : DropItems)
	{
		if (!CheckCanGetItem(DropItem.DropPercent))
		{
			continue;
		}

		FKODropItemMessage ItemMessage;
		ItemMessage.ItemId = DropItem.DropItemName;
		ItemMessage.Count = DropItem.Count;

		UGMRouterSubsystem::BroadcastMessage(
			GetWorld(),
			KOGameplayTags::Event_DropItem,
			FInstancedStruct::Make(ItemMessage)
		);
		OnItemDrop.Broadcast();
	}
	
	ApplyCollectedFromSave();
}

FText AKOItemDropActor::GetInteractionPrompt() const
{
	if (DropItems.Num() <= 0)
	{
		return FText::FromString(TEXT("채집"));
	}

	return UKOItemLibrary::GetDisplayName(this, EKOSlotKind::Item, DropItems[0].DropItemName);
}

bool AKOItemDropActor::CheckCanGetItem(float DropPercent)
{
	float RandomValue=FMath::RandRange(0.f,100.f);
	
	return RandomValue <= DropPercent;
}

void AKOItemDropActor::ApplyCollectedFromSave()
{
	bHasItem = false;
	
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);
}

void AKOItemDropActor::ApplyAvailableFromSave()
{
	bHasItem = true;

	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);
}

void AKOItemDropActor::MarkCollectedForSave()
{
	if (DropSaveId.IsNone())
	{
		return;
	}

	if (UKOSaveSubsystem* SaveSubsystem = UKOSaveSubsystem::Get(this))
	{
		SaveSubsystem->MarkItemDropCollected(DropSaveId);
	}
}





