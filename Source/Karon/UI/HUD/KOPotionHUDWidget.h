// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"
#include "Utility/Messaging/KOGMSInterface.h"
#include "KOPotionHUDWidget.generated.h"

class UImage;
class UCommonTextBlock;
class UTextBlock;
class UKOInventoryComponent;
class APawn;
class UGameplayEffect;
class UAbilitySystemComponent;
class UWidgetSwitcher;
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
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// 표시할 포션의 아이템 태그 (기본값: Item.HealingPotion)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion", Meta = (Categories = "Item"))
	FGameplayTag PotionItemTag;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> PotionStateSwitcher;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> PotionIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> PotionCountText;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> CoolDownOverlay;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CoolDownText;
	
	UPROPERTY(EditDefaultsOnly, Category = "Potion|Cooldown")
	FName CooldownPercentParamName = TEXT("Percent");

	/**
	 * 포션 사용 어빌리티가 CommitAbility 시 적용하는 쿨타임 GameplayEffect 클래스.
	 * SkillQuickSlotEntry와 달리 GameplayTag가 아니라, 이 GE가 ASC에 활성 상태로 붙어있는 동안의
	 * 잔여시간/지속시간을 직접 쿼리해서 쿨타임 UI를 갱신한다.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Potion|Cooldown")
	TSubclassOf<UGameplayEffect> CooldownEffectClass;

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

	UAbilitySystemComponent* GetOwnerASC() const;

	/** CooldownEffectClass가 현재 ASC에 활성 상태인지 조회해 남은 시간/전체 지속시간을 반환. 없으면 false. */
	bool GetCooldownRemainingAndDuration(float& OutRemaining, float& OutDuration) const;

	/** CoolDownOverlay(진행률)/CoolDownText(남은 시간)를 현재 쿨타임 상태로 갱신. */
	void RefreshCooldownVisual();

	FName CachedPotionItemId = NAME_None;

	UPROPERTY()
	TObjectPtr<UKOInventoryComponent> CachedInventoryComponent;

	FGameplayMessageHandle   InventoryChangedHandle;
	FGameplayMessageCallback InventoryChangedCallback;
};