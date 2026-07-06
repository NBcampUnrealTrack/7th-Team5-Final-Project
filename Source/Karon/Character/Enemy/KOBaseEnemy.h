// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
	
protected:
	virtual void BeginPlay() override;
	
	virtual void InitializeAttributes() override;
	
	virtual void OnCharacterDead(AActor* DeathInstigator) override;
	
private:
	UFUNCTION()
	void OnHealthChanged(float OldValue, float NewValue);
	
	void DropItem();
	
	UFUNCTION()
	void OnGroggyBegin();

	
public:
	//TODO: 토큰&티켓 패턴으로 티켓을 받아 공격가능한지 여부(현재는 BP에서 설정)
	UPROPERTY(EditAnywhere)
	bool bCanAttack=true;
	
	UPROPERTY(EditAnywhere)
	bool bCanPatrol=true;
	
	//Projectile
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UStaticMesh> ProjectileMesh;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> ProjectileDamageEffectClass;
	
	UPROPERTY(EditAnywhere)
	FVector ProjectileScale=FVector(1.f,1.f,1.f);
	
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
	
	
public:
	FOnGameplayAbilityEnd OnGameplayAbilityEnd;
	FOnCharacterEvent OnCharacterReset;
	FOnUIChangeEvent OnHPChangedEvent;
	FOnUIVisibleEvent OnBattleEvent;
	FOnUIVisibleEvent OnLockOnEvent;
	FOnUIVisibleEvent OnParriedEvent;
	FOnTriggerEvent OnHitEvent;
	FOnTriggerEvent OnCounterAttackEvent;
	
	
	UPROPERTY(BlueprintAssignable)
	FOnCharacterDeadEvent OnEnemyDead;

	
};
