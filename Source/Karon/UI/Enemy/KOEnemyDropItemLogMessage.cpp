// Fill out your copyright notice in the Description page of Project Settings.


#include "KOEnemyDropItemLogMessage.h"

#include "AbilitySystem/Tag/Event/KOGameplayTags_Event.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Items/KOItemLibrary.h"
#include "Utility/Messaging/KOMessageTypes.h"

void UKOEnemyDropItemLogMessage::NativeConstruct()
{
	Super::NativeConstruct();
	
}

void UKOEnemyDropItemLogMessage::NativeDestruct()
{
	Super::NativeDestruct();
}

void UKOEnemyDropItemLogMessage::SetLogMessage(FName ItemId, int32 Count)
{
	FText CachedDisplayName = UKOItemLibrary::GetDisplayName(this, EKOSlotKind::Item, ItemId);
	UTexture2D* CachedIcon        = UKOItemLibrary::GetIcon(this, EKOSlotKind::Item, ItemId);
	
	if (ItemIconImage)
	{
		ItemIconImage->SetBrushFromTexture(CachedIcon);
	}
	if (ItemGetTextBlock)
	{
		FText LogText = FText::Format(
			FText::FromString(TEXT("{0} x {1} 획득하였습니다")), 
			CachedDisplayName,
			FText::AsNumber(Count)
		);

		ItemGetTextBlock->SetText(LogText);	
	}
	
}
