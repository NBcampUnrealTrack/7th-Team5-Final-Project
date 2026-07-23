// Copyright Karon Team 5. All Rights Reserved.

#include "KOGA_UsePotion.h"
#include "Component/Inventory/KOInventoryComponent.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "GameFramework/PlayerController.h"

UKOGA_UsePotion::UKOGA_UsePotion()
{
	SetAssetTags(FGameplayTagContainer(KOGameplayTags::Input_Ability_Utility_UsePotion));
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

UKOInventoryComponent* UKOGA_UsePotion::ResolveInventoryComponent(const FGameplayAbilityActorInfo* ActorInfo)
{
	if (ActorInfo == nullptr)
	{
		return nullptr;
	}

	// UKOInventoryComponent는 Pawn이 아니라 PlayerController에 붙어있다 (KOPlayerController.cpp 참고).
	if (APlayerController* PC = ActorInfo->PlayerController.Get())
	{
		if (UKOInventoryComponent* InventoryComponent = PC->FindComponentByClass<UKOInventoryComponent>())
		{
			return InventoryComponent;
		}
	}

	if (AActor* Avatar = ActorInfo->AvatarActor.Get())
	{
		return Avatar->FindComponentByClass<UKOInventoryComponent>();
	}

	return nullptr;
}

bool UKOGA_UsePotion::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	AActor* Avatar = ActorInfo->AvatarActor.Get();
	if (Avatar == nullptr)
	{
		return false;
	}

	UKOInventoryComponent* InventoryComponent = ResolveInventoryComponent(ActorInfo);
	if (InventoryComponent == nullptr)
	{
		return false;
	}

	// PotionTag(Item.HealingPotion)를 인벤토리가 사용하는 ItemId(FName)로 역조회
	const UKOLoadSubsystem* LoadSubsystem = UKOLoadSubsystem::Get(Avatar);
	const FName PotionItemId = LoadSubsystem ? LoadSubsystem->FindItemIdByTag(PotionTag) : NAME_None;

	if (PotionItemId.IsNone() || InventoryComponent->HasEnoughItems(PotionItemId, 1) == false)
	{
		return false;
	}

	return true;
}

void UKOGA_UsePotion::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AActor* Avatar = ActorInfo->AvatarActor.Get();
	UKOInventoryComponent* InventoryComponent = ResolveInventoryComponent(ActorInfo);
	const UKOLoadSubsystem* LoadSubsystem = Avatar ? UKOLoadSubsystem::Get(Avatar) : nullptr;
	const FName PotionItemId = LoadSubsystem ? LoadSubsystem->FindItemIdByTag(PotionTag) : NAME_None;

	// 포션 소모 (인벤토리에서 1개 차감). 실패 시 회복 효과를 적용하지 않고 종료.
	if (InventoryComponent == nullptr || PotionItemId.IsNone() ||
		InventoryComponent->TryRemoveItem(PotionItemId, 1) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ApplyEffectToSelf(HealingEffectClass);

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
