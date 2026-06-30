#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Items/KOItemSlot.h"
#include "Component/Inventory/KOEquipmentComponent.h"
#include "KOSaveGame.generated.h"

UCLASS()
class KARON_API UKOSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// 플레이어 위치
	UPROPERTY()
	bool bHasPlayerTransform = false;

	UPROPERTY()
	FTransform PlayerTransform = FTransform::Identity;

	// 인벤토리
	UPROPERTY()
	TArray<FKOItemSlot> InventorySlots;

	// 장착 무기
	UPROPERTY()
	bool bHasEquippedWeapon = false;

	UPROPERTY()
	FName EquippedWeaponItemId = NAME_None;

	UPROPERTY()
	EWeaponSlot EquippedWeaponSlot = EWeaponSlot::Holster;
};