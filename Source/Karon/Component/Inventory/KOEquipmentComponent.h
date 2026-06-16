#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/Character/KOGrantSet.h"
#include "KOEquipmentComponent.generated.h"

class UKOWeaponDefinition;
class AKOWeaponBase;

UENUM(BlueprintType)
enum class EWeaponSlot : uint8
{
	Hand    UMETA(DisplayName = "Hand"),    // 손에 뽑아 든 상태
	Holster UMETA(DisplayName = "Holster"), // 칼집/등에 꽂힌 상태
};



UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
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
	
protected: 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> SkeletalMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> BodyMesh;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UAnimInstance> DefaultAnimLayerClass;
};
