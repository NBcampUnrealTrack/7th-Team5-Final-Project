#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "KOWeaponDefinition.generated.h"

class UKOGrantSet;

USTRUCT(BlueprintType)
struct KARON_API FWeaponAnimationSet
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> DrawMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> SheatheMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UAnimInstance> WeaponABP_Sheathed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UAnimInstance> WeaponABP_Carrying;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UAnimInstance> WeaponABP_Combat;
};

USTRUCT(BlueprintType)
struct KARON_API FKOAttachSocket
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName SocketName = NAME_None;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FTransform Transform = FTransform::Identity;
};

USTRUCT(BlueprintType)
struct KARON_API FWeaponAttachSockets
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FKOAttachSocket EquipSocket; 
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FKOAttachSocket UnequipSocket; 
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FKOAttachSocket GripSocket;
};

USTRUCT(BlueprintType)
struct KARON_API FWeaponTraceSockets
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName TraceStartSocket = TEXT("TraceStart");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName TraceEndSocket = TEXT("TraceEnd"); 
};

UCLASS(BlueprintType)
class KARON_API UKOWeaponDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Type")
	FName WeaponName = NAME_None;

	// 무기 스태틱 메시
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Visuals")
	TSoftObjectPtr<UStaticMesh> WeaponMesh;
	
	// 장착 시 부여할 어빌리티, GE 묶음
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon| Grant")
	TObjectPtr<UKOGrantSet> GrantedSet;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FWeaponAnimationSet WeaponAnimationSet;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FWeaponAttachSockets AttachSockets;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FWeaponTraceSockets TraceSockets;
};
