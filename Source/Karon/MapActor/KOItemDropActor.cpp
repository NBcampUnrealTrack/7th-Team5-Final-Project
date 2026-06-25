// Fill out your copyright notice in the Description page of Project Settings.


#include "KOItemDropActor.h"

#include "AbilitySystem/Tag/Event/KOGameplayTags_Event.h"
#include "Items/KOItemLibrary.h"
#include "Items/KOItemSlot.h"
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
	FKODropItemMessage ItemMessage;
	ItemMessage.ItemId=ItemInfo.DropItemName;
	ItemMessage.Count=ItemInfo.Count;
			
	UGMRouterSubsystem::BroadcastMessage(GetWorld(),
		KOGameplayTags::Event_DropItem,
		FInstancedStruct::Make(ItemMessage));
	
	bHasItem=false;
	Destroy();
}

FText AKOItemDropActor::GetInteractionPrompt() const
{
	FText CachedDisplayName = UKOItemLibrary::GetDisplayName(this, EKOSlotKind::Item, ItemInfo.DropItemName);

	
	return CachedDisplayName;
}




