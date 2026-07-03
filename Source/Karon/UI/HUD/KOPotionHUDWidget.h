// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"
#include "Utility/Messaging/KOGMSInterface.h"
#include "KOPotionHUDWidget.generated.h"

class UImage;
class UCommonTextBlock;
class UKOInventoryComponent;
class APawn;
struct FInstancedStruct;

/**
 * HUD에 상시로 표시되는 회복 포션 아이콘 + 보유 수량 위젯.
 * 소유 폰의 인벤토리를 조회해 초기값을 세팅하고, 이후 인벤토리 변경 메시지를 구독해 갱신한다.
 */
UCLASS()
class KARON_API UKOPotionHUDWidget : public UCommonUserWidget, public IKOGMSInterface
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// 표시할 포션의 아이템 태그 (기본값: Item.HealingPotion)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion", Meta = (Categories = "Item"))
	FGameplayTag PotionItemTag;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> PotionIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> PotionCountText;

private:
	// NativeConstruct 시점에는 OwningPlayer가 아직 폰을 소유하지 않았을 수 있어,
	// 초기 1회 시도 + OnPossessedPawnChanged 재시도의 두 경로로 캐시를 채운다.
	void InitializeFromPawn(APawn* Pawn);

	void RefreshPotionIcon();
	void RefreshPotionCount();

	UFUNCTION()
	void HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);

	UFUNCTION()
	void HandleInventoryChangedMessage(FGameplayTag Channel, const FInstancedStruct& Payload);

	FName CachedPotionItemId = NAME_None;

	UPROPERTY()
	TObjectPtr<UKOInventoryComponent> CachedInventoryComponent;

	FGameplayMessageHandle   InventoryChangedHandle;
	FGameplayMessageCallback InventoryChangedCallback;
};