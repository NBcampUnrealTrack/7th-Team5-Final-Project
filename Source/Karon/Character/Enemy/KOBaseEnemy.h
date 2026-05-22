// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/KOCharacterBase.h"
#include "KOBaseEnemy.generated.h"

class UKOAnimNotifyComponent;
class UKOEnemyDataAsset;
class UGameplayAbility;
class UKOCombatSet;

UCLASS()
class KARON_API AKOBaseEnemy : public AKOCharacterBase
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AKOBaseEnemy(const FObjectInitializer& ObjectInitializer);
	
	void SetupEnemy(UKOEnemyDataAsset);
	
	FORCEINLINE UKOAnimNotifyComponent* GetAnimNotifyComponent(){return AnimNotifyComponent;}

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
private:
	void GiveDefaultAbilities();
	
protected:
	UPROPERTY()
	TObjectPtr<UKOCombatSet> CombatSet;
	
	UPROPERTY()
	TObjectPtr<UKOAnimNotifyComponent> AnimNotifyComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> WeaponMeshComponent;
	
};
