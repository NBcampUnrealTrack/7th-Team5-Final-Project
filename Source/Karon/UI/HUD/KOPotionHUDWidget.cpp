// Copyright Karon Team 5. All Rights Reserved.

#include "KOPotionHUDWidget.h"
#include "Component/Inventory/KOInventoryComponent.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Utility/Messaging/KOMessageTypes.h"
#include "StructUtils/InstancedStruct.h"
#include "CommonTextBlock.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffect.h"
#include "Materials/MaterialInstanceDynamic.h"

void UKOPotionHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (PotionItemTag.IsValid() == false)
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

void UKOPotionHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	RefreshCooldownVisual();
}

void UKOPotionHUDWidget::InitializeFromPawn(APawn* Pawn)
{
	// 다른 인벤토리 참조 위젯들과 동일하게 PC를 먼저 찾고, 없으면 Pawn을 폴백으로 확인한다.
	CachedInventoryComponent = nullptr;

	if (APlayerController* OwningPC = GetOwningPlayer())
	{
		CachedInventoryComponent = OwningPC->FindComponentByClass<UKOInventoryComponent>();
	}

	if (CachedInventoryComponent == nullptr && Pawn)
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
	if (PotionIcon == nullptr || CachedPotionItemId.IsNone())
	{
		return;
	}

	const UKOLoadSubsystem* LoadSubsystem = UKOLoadSubsystem::Get(this);
	UTexture2D* IconTexture = LoadSubsystem ? LoadSubsystem->ResolveItemIcon(CachedPotionItemId) : nullptr;
	if (!IconTexture)
	{
		return;
	}

	PotionIcon->SetBrushResourceObject(IconTexture);
}

void UKOPotionHUDWidget::RefreshPotionCount()
{
	const int32 Count = (CachedInventoryComponent && !CachedPotionItemId.IsNone())
		? CachedInventoryComponent->GetCountOf(CachedPotionItemId) : 0;

	if (PotionCountText)
	{
		PotionCountText->SetText(FText::AsNumber(Count));
	}

	if (PotionStateSwitcher)
	{
		const int32 SwitcherIndex = Count > 0 ? 1 : 0;
		PotionStateSwitcher->SetActiveWidgetIndex(SwitcherIndex);
	}
}

void UKOPotionHUDWidget::HandleInventoryChangedMessage(FGameplayTag Channel, const FInstancedStruct& Payload)
{
	const FKOInventoryChangedMessage* Msg = Payload.GetPtr<FKOInventoryChangedMessage>();
	if (Msg == nullptr || CachedPotionItemId.IsNone())
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

UAbilitySystemComponent* UKOPotionHUDWidget::GetOwnerASC() const
{
	APlayerController* PC = GetOwningPlayer();
	if (PC == nullptr)
	{
		return nullptr;
	}

	APawn* Pawn = PC->GetPawn();
	if (Pawn == nullptr)
	{
		return nullptr;
	}

	return UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn);
}

bool UKOPotionHUDWidget::GetCooldownRemainingAndDuration(float& OutRemaining, float& OutDuration) const
{
	if (CooldownEffectClass == nullptr)
	{
		return false;
	}

	UAbilitySystemComponent* ASC = GetOwnerASC();
	if (ASC == nullptr)
	{
		return false;
	}

	FGameplayEffectQuery Query;
	Query.EffectDefinition = CooldownEffectClass;

	const TArray<TPair<float, float>> TimesAndDurations = ASC->GetActiveEffectsTimeRemainingAndDuration(Query);
	if (TimesAndDurations.IsEmpty())
	{
		return false;
	}

	OutRemaining = FMath::Max(0.f, TimesAndDurations[0].Key);
	OutDuration  = TimesAndDurations[0].Value;
	return true;
}

void UKOPotionHUDWidget::RefreshCooldownVisual()
{
	float Remaining = 0.f;
	float Duration  = 0.f;
	const bool bOnCooldown = GetCooldownRemainingAndDuration(Remaining, Duration) && Duration > 0.f;

	if (CoolDownOverlay)
	{
		CoolDownOverlay->SetVisibility(bOnCooldown ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);

		if (bOnCooldown)
		{
			if (UMaterialInstanceDynamic* MID = CoolDownOverlay->GetDynamicMaterial())
			{
				const float Percent = FMath::Clamp(Remaining / Duration, 0.f, 1.f);
				MID->SetScalarParameterValue(CooldownPercentParamName, Percent);
			}
		}
	}

	if (CoolDownText)
	{
		if (bOnCooldown)
		{
			CoolDownText->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), Remaining)));
			CoolDownText->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			CoolDownText->SetText(FText::GetEmpty());
			CoolDownText->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}