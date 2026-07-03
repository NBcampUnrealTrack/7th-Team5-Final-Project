// Copyright Karon Team 5. All Rights Reserved.

#include "KOPotionHUDWidget.h"
#include "Component/Inventory/KOInventoryComponent.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Utility/Messaging/KOMessageTypes.h"
#include "StructUtils/InstancedStruct.h"
#include "CommonTextBlock.h"
#include "Components/Image.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

void UKOPotionHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!PotionItemTag.IsValid())
	{
		PotionItemTag = KOGameplayTags::Item_HealingPotion;
	}

	// HUD가 먼저 생성되고 폰이 나중에 빙의되는 순서일 수 있어, 가능하면 즉시 초기화하고
	// 그렇지 않다면 OnPossessedPawnChanged로 폰이 준비된 시점에 다시 초기화한다.
	InitializeFromPawn(GetOwningPlayerPawn());

	if (APlayerController* OwningPC = GetOwningPlayer())
	{
		OwningPC->OnPossessedPawnChanged.AddDynamic(this, &UKOPotionHUDWidget::HandlePossessedPawnChanged);
	}

	InventoryChangedCallback.BindDynamic(this, &UKOPotionHUDWidget::HandleInventoryChangedMessage);
	InventoryChangedHandle = Subscribe(KOGameplayTags::Data_Message_Inventory_Changed, InventoryChangedCallback);
}

void UKOPotionHUDWidget::NativeDestruct()
{
	if (APlayerController* OwningPC = GetOwningPlayer())
	{
		OwningPC->OnPossessedPawnChanged.RemoveDynamic(this, &UKOPotionHUDWidget::HandlePossessedPawnChanged);
	}

	Unsubscribe(InventoryChangedHandle);
	InventoryChangedCallback.Clear();

	Super::NativeDestruct();
}

void UKOPotionHUDWidget::InitializeFromPawn(APawn* Pawn)
{
	// 다른 인벤토리 참조 위젯들과 동일하게 PC를 먼저 찾고, 없으면 Pawn을 폴백으로 확인한다.
	CachedInventoryComponent = nullptr;

	if (APlayerController* OwningPC = GetOwningPlayer())
	{
		CachedInventoryComponent = OwningPC->FindComponentByClass<UKOInventoryComponent>();
	}

	if (!CachedInventoryComponent && Pawn)
	{
		CachedInventoryComponent = Pawn->FindComponentByClass<UKOInventoryComponent>();
	}

	if (CachedPotionItemId.IsNone())
	{
		if (const UKOLoadSubsystem* LoadSubsystem = UKOLoadSubsystem::Get(this))
		{
			CachedPotionItemId = LoadSubsystem->FindItemIdByTag(PotionItemTag);
		}
	}

	RefreshPotionIcon();
	RefreshPotionCount();
}

void UKOPotionHUDWidget::HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	InitializeFromPawn(NewPawn);
}

void UKOPotionHUDWidget::RefreshPotionIcon()
{
	if (!PotionIcon || CachedPotionItemId.IsNone())
	{
		return;
	}

	const UKOLoadSubsystem* LoadSubsystem = UKOLoadSubsystem::Get(this);
	UTexture2D* IconTexture = LoadSubsystem ? LoadSubsystem->ResolveItemIcon(CachedPotionItemId) : nullptr;
	if (!IconTexture)
	{
		return;
	}

	FSlateBrush SlateBrush;
	SlateBrush.SetResourceObject(IconTexture);
	PotionIcon->SetBrush(SlateBrush);
}

void UKOPotionHUDWidget::RefreshPotionCount()
{
	if (!PotionCountText)
	{
		return;
	}

	const int32 Count = (CachedInventoryComponent && !CachedPotionItemId.IsNone())
		? CachedInventoryComponent->GetCountOf(CachedPotionItemId)
		: 0;

	PotionCountText->SetText(FText::AsNumber(Count));
}

void UKOPotionHUDWidget::HandleInventoryChangedMessage(FGameplayTag Channel, const FInstancedStruct& Payload)
{
	const FKOInventoryChangedMessage* Msg = Payload.GetPtr<FKOInventoryChangedMessage>();
	if (!Msg || CachedPotionItemId.IsNone())
	{
		return;
	}

	// LoadSlotsFromSave 등 전체 갱신 브로드캐스트는 ItemId가 NAME_None으로 온다
	if (!Msg->ItemId.IsNone() && Msg->ItemId != CachedPotionItemId)
	{
		return;
	}

	RefreshPotionCount();
}