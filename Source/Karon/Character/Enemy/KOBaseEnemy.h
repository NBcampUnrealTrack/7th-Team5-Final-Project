// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/KOCharacterBase.h"
#include "KOBaseEnemy.generated.h"

class UKOAnimNotifyComponent;
class UKOEnemyDataAsset;
class UGameplayAbility;
class UKOCombatSet;
struct FOnAttributeChangeData;

DECLARE_DELEGATE(FOnGameplayAbilityEnd)
DECLARE_DELEGATE(FOnCharacterEvent)

UCLASS()
class KARON_API AKOBaseEnemy : public AKOCharacterBase
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AKOBaseEnemy(const FObjectInitializer& ObjectInitializer);
	
	void SetupEnemy(UKOEnemyDataAsset);
	FORCEINLINE UKOAnimNotifyComponent* GetAnimNotifyComponent(){return AnimNotifyComponent;}
	FVector GetSocketLocation();
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	

private:
	void GiveDefaultAbilities();
	void OnHitCallback(const FOnAttributeChangeData& Data);

	
public:
	//TODO: 토큰&티켓 패턴으로 티켓을 받아 공격가능한지 여부(현재는 BP에서 설정)
	UPROPERTY(EditAnywhere)
	bool bCanAttack=true;
	
protected:
	UPROPERTY()
	TObjectPtr<UKOCombatSet> CombatSet;
	
	UPROPERTY()
	TObjectPtr<UKOAnimNotifyComponent> AnimNotifyComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> WeaponMeshComponent;
	FName HandSocketName=TEXT("hand_r_Socket");
	FName WeaponSocketName=TEXT("Weapon_Socket");
		
private:

	
	//Delegates
public:
	FOnGameplayAbilityEnd OnGameplayAbilityEnd;
	FOnCharacterEvent OnCharacterHit;
	FOnCharacterEvent OnCharacterDead;
	FOnCharacterEvent OnCharacterReset;
	
	

};
