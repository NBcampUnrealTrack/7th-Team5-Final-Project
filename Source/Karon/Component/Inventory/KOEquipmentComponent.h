#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/Character/KOGrantSet.h"
#include "Data/KODataTableTypes.h"
#include "KOEquipmentComponent.generated.h"

class UKOWeaponDefinition;
class AKOWeaponBase;
class UGameplayEffect;

UENUM(BlueprintType)
enum class EWeaponSlot : uint8
{
	Hand    UMETA(DisplayName = "Hand"),    // 손에 뽑아 든 상태
	Holster UMETA(DisplayName = "Holster"), // 칼집/등에 꽂힌 상태
};



UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class KARON_API UKOEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UKOEquipmentComponent();
	
	virtual void BeginPlay() override;

	// 무기 정의를 받아서 액터 스폰, 소켓 어태치, GAS 부여
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void EquipWeapon(UKOWeaponDefinition* Def);

	// 현재 무기 해제: GAS 회수, 액터 제거
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void UnequipWeapon();
	
	// 뽑기: 칼집(Holster) → 손(Hand)
	void DrawWeapon();

	// 넣기: 손(Hand) → 칼집(Holster)
	void SheatheWeapon();
	
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void SetBodyMesh(USkeletalMeshComponent* NewSkeletalMesh) { BodyMesh = NewSkeletalMesh; }
	
	UFUNCTION(BlueprintPure, Category = "Equipment")
	bool HasWeapon() const { return CurrentWeaponActor != nullptr; }

	UFUNCTION(BlueprintPure, Category = "Equipment")
	bool IsWeaponDrawn() const { return CurrentWeaponSlot == EWeaponSlot::Hand; }

	UFUNCTION(BlueprintPure, Category = "Equipment")
	UKOWeaponDefinition* GetCurrentWeaponConfig() const { return CurrentWeaponConfig; }
	
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void ToggleWeaponDrawState();
	
	FName GetCurrentWeaponItemId() const { return CurrentWeaponItemId; }
	EWeaponSlot GetCurrentWeaponSlot() const { return CurrentWeaponSlot; }

	// 무기 장착
	bool EquipWeaponFromItem(FName InWeaponItemId, UKOWeaponDefinition* Def);
	
	// 방어구 장착
	bool EquipArmorFromItem(EKOEquipmentSlotType SlotType, FName ItemId);

	// 방어구 장착 해제
	void UnequipArmor(EKOEquipmentSlotType SlotType);

	UFUNCTION(BlueprintPure, Category = "Equipment|Armor")
	FName GetEquippedArmorItemId(EKOEquipmentSlotType SlotType) const;

	int32 GetTotalArmorDefense() const { return TotalArmorDefense; }
	const TMap<EKOEquipmentSlotType, FName>& GetEquippedArmorItemIds() const {return EquippedArmorItemIds;}

	// 방어력 총합 계산
	void RecalculateArmorDefense();
	
	// 세이브 로드
	bool RestoreWeaponFromSave(FName InWeaponItemId, UKOWeaponDefinition* Def, EWeaponSlot SavedSlot);
	void LoadArmorFromSave(const TMap<EKOEquipmentSlotType, FName>& SavedArmorItemIds);
	
protected:
	void SetWeaponSlot(EWeaponSlot NewSlot);
	
public:
	UPROPERTY()
	TObjectPtr<UKOWeaponDefinition> CurrentWeaponConfig;

	FKOAbilitySetHandles ActiveHandles;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<AKOWeaponBase> CurrentWeaponActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EWeaponSlot CurrentWeaponSlot = EWeaponSlot::Holster;
	
	// 무기
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
	FName CurrentWeaponItemId = NAME_None;
	
	// 방어구
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment|Armor")
	TMap<EKOEquipmentSlotType, FName> EquippedArmorItemIds;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment|Armor")
	int32 TotalArmorDefense = 0;

	// 장착 방어구 총합을 Defense 어트리뷰트(Data.Attribute.Combat.Defense, SetByCaller)에 반영하는 GE.
	// 무한 지속 + Add 방식이어야 하며, 방어구가 바뀔 때마다 제거 후 새 총합으로 재적용된다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment|Armor")
	TSubclassOf<UGameplayEffect> ArmorDefenseEffectClass;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> SkeletalMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> BodyMesh;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UAnimInstance> DefaultAnimLayerClass;
	
private:
	void SyncWeaponDrawnTagToASC();

	// TotalArmorDefense를 ArmorDefenseEffectClass GE로 ASC에 반영 (기존 적용분은 제거 후 재적용).
	void ApplyArmorDefenseEffect();

	FActiveGameplayEffectHandle ArmorDefenseEffectHandle;
};
