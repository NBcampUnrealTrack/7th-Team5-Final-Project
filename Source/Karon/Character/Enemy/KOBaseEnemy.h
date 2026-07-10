// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Attribute/KOCombatSet.h"
#include "Character/KOCharacterBase.h"
#include "KOBaseEnemy.generated.h"


class UKOGroggySet;
class UKOEnemyDataSubsystem;
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
DECLARE_DELEGATE_OneParam(FOnUIVisibleEvent,bool bIsBattle)
DECLARE_DELEGATE_OneParam(FOnTriggerEvent,bool bIsTriggered)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCharacterDeadEvent);

UCLASS()
class KARON_API AKOBaseEnemy : public AKOCharacterBase
{
	GENERATED_BODY()

public:
	AKOBaseEnemy(const FObjectInitializer& ObjectInitializer);
	
	void SetupEnemy(UKOEnemyDataSubsystem* DataSubsystem,int32 Level);
	FVector GetSocketLocation();
	float GetAttackPoint();
	void OnBattleChanged(bool bIsBattle);
	void ChangeLockOnGroggy(bool bIsGroggied);
	
	// 세이브 로드
	FName GetMonsterSaveId() const { return MonsterSaveId; }
	bool IsDeadForSave() const { return bDeadForSave; }
	
	FName GetClusterSaveIdForSave() const { return ClusterSaveId; }
	int32 GetEnemyLevelForSave() const { return EnemyLevel; }

	void SetMonsterSaveInfoForLoad(FName InClusterSaveId, FName InMonsterSaveId);
	void RestoreMonsterFromSave(const FTransform& SavedTransform);
	virtual void OnCharacterDead(AActor* DeathInstigator) override;
	
	FORCEINLINE float GetAttackPower() const
	{if (CombatSet){return CombatSet->GetAttackPower();} return 0.f;}
	
protected:
	virtual void BeginPlay() override;
	virtual void InitializeAttributes() override;
	
	
private:
	UFUNCTION()
	void OnHealthChanged(float OldValue, float NewValue);
	
	void DropItem();
	
	UFUNCTION()
	void OnGroggyBegin();

	
public:
	bool bCanAttack=true;
	
	UPROPERTY(EditAnywhere)
	bool bCanPatrol=true;
	
	//Projectile
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile")
	TObjectPtr<UStaticMesh> ProjectileMesh;
	
	UPROPERTY(EditAnywhere, Category="Projectile")
	TSubclassOf<UGameplayEffect> ProjectileDamageEffectClass;
	
	UPROPERTY(EditAnywhere, Category="Projectile")
	FVector ProjectileScale=FVector(1.f,1.f,1.f);
	
	UPROPERTY(EditAnywhere, Category="Projectile")
	FGameplayTag ProjectileTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	class UNiagaraSystem* ImpactEffect;
	
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
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag EnemyNameTag;
	
	UPROPERTY(EditDefaultsOnly,Category="Attribute")
	int32 EnemyLevel=1;
	
	UPROPERTY(EditDefaultsOnly,Category="Attribute")
	float MaxGroggyHealth=50.f;
	
	UPROPERTY()
	AActor* TargetActor=nullptr;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> WeaponMeshComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UWidgetComponent> EnemyHPBarWidgetComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UWidgetComponent> EnemyLockOnWidgetComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UWidgetComponent> EnemyParriedWidgetComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attribute | Groggy")
	TObjectPtr<UKOGroggySet> GroggySet;
	
	FName HandSocketName=TEXT("hand_r_Socket");
	FName WeaponSocketName=TEXT("Weapon_Socket");
	FName SkeletonSocketName=TEXT("Skeleton_Socket");
	FName LockOnSocketName=FName("LockOnSocket");
	
	FVector LocalLockOnInitialLocation=FVector(0,0,100.f);;
	FVector LocalLockOnOffset=FVector(0,40.f,-80.f);
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "KO|Save")
	FName ClusterSaveId = NAME_None;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "KO|Save")
	FName MonsterSaveId = NAME_None;

	UPROPERTY()
	bool bDeadForSave = false;
	
private:
	bool bHadSendEvent=false;
	
public:
	FOnGameplayAbilityEnd OnGameplayAbilityEnd;
	FOnCharacterEvent OnCharacterReset;
	FOnUIChangeEvent OnHPChangedEvent;
	FOnUIVisibleEvent OnBattleEvent;
	FOnUIVisibleEvent OnLockOnEvent;
	FOnUIVisibleEvent OnParriedEvent;
	FOnTriggerEvent OnHitEvent;
	FOnTriggerEvent OnCounterAttackEvent;
	FOnTriggerEvent OnCanAttackEvent;
	FOnTriggerEvent OnHalfHealthEvent;
	
	UPROPERTY(BlueprintAssignable)
	FOnCharacterDeadEvent OnEnemyDead;

	
};
