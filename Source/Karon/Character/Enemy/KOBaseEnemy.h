// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/KOCharacterBase.h"
#include "KOBaseEnemy.generated.h"

class UGameplayEffect;
class UWidgetComponent;
class UKOAnimNotifyComponent;
class UKOEnemyDataAsset;
class UGameplayAbility;
class UKOCombatSet;
struct FOnAttributeChangeData;

DECLARE_DELEGATE(FOnGameplayAbilityEnd)
DECLARE_DELEGATE(FOnCharacterEvent)
DECLARE_DELEGATE_TwoParams(FOnUIChangeEvent, float ProgressPercent,float Damage)
DECLARE_DELEGATE_OneParam(FOnUIBattleEvent,bool bIsBattle)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCharacterDeadEvent);

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
	void OnBattleChanged(bool bIsBattle);
	
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
	
	//TODO: 원거리 몬스터 전용(컴포넌트로 분리)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UStaticMesh> ProjectileMesh;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> ProjectileDamageEffectClass;
	
	UPROPERTY(EditAnywhere)
	FVector ProjectileScale=FVector(1.f,1.f,1.f);
	
	//TODO: 비동기 로드로 세팅
	UPROPERTY(EditDefaultsOnly,Category="Attribute")
	float EnemyAttackRadius=150.f;
	
	UPROPERTY(EditDefaultsOnly,Category="Attribute")

	bool bIsEnemyLongRange=false;
	
	UPROPERTY(EditDefaultsOnly,Category="Attribute")
	float EnemySpeed=400.f;
	
	UPROPERTY(EditDefaultsOnly,Category="Attribute")
	float EnemyStrafeSpeed=200.f;
	
	UPROPERTY(EditDefaultsOnly,Category="Attribute")
	float EnemyAttackDelayTime=0.5f;

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
	FName SkeletonSocketName=TEXT("Skeleton_Socket");
		
private:

	
	//Delegates
public:
	FOnGameplayAbilityEnd OnGameplayAbilityEnd;
	
	FOnCharacterEvent OnCharacterHit;

	FOnCharacterEvent OnCharacterReset;
	
	FOnUIChangeEvent OnHPChangedEvent;
	FOnUIBattleEvent OnBattleEvent;
	
	UPROPERTY(BlueprintAssignable)
	FOnCharacterDeadEvent OnEnemyDead;
};
