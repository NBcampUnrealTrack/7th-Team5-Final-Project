#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KOWeaponBase.generated.h"

class UKOWeaponDefinition;
class USkeletalMeshComponent;

UCLASS()
class KARON_API AKOWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	AKOWeaponBase();

	void InitializeWeapon(const UKOWeaponDefinition* Def);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	TObjectPtr<const UKOWeaponDefinition> WeaponDef;
};
