// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/KOGameplayAbilityBase.h"
#include "KOGA_UsePotion.generated.h"

class UKOInventoryComponent;

/**
 *
 */
UCLASS()
class KARON_API UKOGA_UsePotion : public UKOGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UKOGA_UsePotion();

	// 능력을 켤 수 있는 상태인지 검사 (포션 개수 체크 등)
	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr
	) const override;

	// 실제 능력이 발동했을 때 실행되는 로직
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

protected:
	// 인벤토리에서 식별할 포션 아이템의 태그 (Item_HealingPotion)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion")
	FGameplayTag PotionTag;

	// 체력을 채워줄 GameplayEffect 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion")
	TSubclassOf<UGameplayEffect> HealingEffectClass;

private:
	// UKOInventoryComponent는 Pawn이 아니라 PlayerController에 붙어있어 그쪽을 먼저 확인하고,
	// 없으면 아바타 액터(폰)를 폴백으로 확인한다 (KOPlayerController.cpp 참고).
	static UKOInventoryComponent* ResolveInventoryComponent(const FGameplayAbilityActorInfo* ActorInfo);
};
