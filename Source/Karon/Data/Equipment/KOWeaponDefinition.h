#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "KOWeaponDefinition.generated.h"

class UKOGrantSet;

USTRUCT(BlueprintType)
struct KARON_API FWeaponBaseStats
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float BaseATK = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float AttackSpeed = 1.f;
};

UCLASS(BlueprintType)
class KARON_API UKOWeaponDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 장착 슬롯 (Weapon.Slot.Primary 등)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Slot")
	FGameplayTag SlotTag;

	// 애니 레이어 전환용 (Weapon.Type.Sword 등)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Type")
	FGameplayTag WeaponTypeTag;

	// 무기 스태틱 메시
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Visuals")
	TSoftObjectPtr<UStaticMesh> WeaponMesh;

	// 뽑은 상태: 손 소켓 (예: "hand_r")
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Visuals")
	FName EquipSocket = TEXT("hand_r");

	// 넣은 상태: 칼집/등 소켓 (예: "spine_02")
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Visuals")
	FName UnEquipSocket = TEXT("spine_02");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
	FWeaponBaseStats BaseStats;

	// 장착 시 부여할 어빌리티, GE 묶음
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|GAS")
	TObjectPtr<UKOGrantSet> GrantedSet;

	// 뽑기 몽타주 (칼집 → 손)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Animation")
	TObjectPtr<UAnimMontage> DrawMontage;

	// 넣기 몽타주 (손 → 칼집)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Animation")
	TObjectPtr<UAnimMontage> SheatheMontage;
};
