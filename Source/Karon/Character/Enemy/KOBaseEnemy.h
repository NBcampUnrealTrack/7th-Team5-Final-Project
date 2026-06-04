// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/KOCharacterBase.h"
#include "KOBaseEnemy.generated.h"

class UWidgetComponent;
class UKOAnimNotifyComponent;
class UKOEnemyDataAsset;
class UGameplayAbility;
class UKOCombatSet;
struct FOnAttributeChangeData;

DECLARE_DELEGATE(FOnGameplayAbilityEnd)
DECLARE_DELEGATE(FOnCharacterEvent)
DECLARE_DELEGATE_OneParam(FOnHUDChangeEvent, float ProgressPercent)

UCLASS()
class KARON_API AKOBaseEnemy : public AKOCharacterBase
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AKOBaseEnemy(const FObjectInitializer& ObjectInitializer);
	
	void SetupEnemy(UKOEnemyDataAsset);
	FVector GetSocketLocation();
	float GetAttackPoint();
	
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
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> WeaponMeshComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UWidgetComponent> EnemyHPBarWidgetComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;
	

	FName HandSocketName=TEXT("hand_r_Socket");
	FName WeaponSocketName=TEXT("Weapon_Socket");
		
private:

	
	//Delegates
public:
	FOnGameplayAbilityEnd OnGameplayAbilityEnd;
	
	FOnCharacterEvent OnCharacterHit;
	FOnCharacterEvent OnCharacterDead;
	FOnCharacterEvent OnCharacterReset;
	
	FOnHUDChangeEvent OnHPChanged;
};
