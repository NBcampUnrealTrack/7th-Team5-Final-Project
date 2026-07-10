#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Character.h"

#include "KOCharacterBase.generated.h"

struct FInstancedStruct;
class UKOAbilitySystemComponent; 
class UKOEquipmentComponent;
class UKOHealthSet;
class UKOMovementSet;
class UKOCombatSet;
class UKOGuardSet;

UCLASS()
class KARON_API AKOCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AKOCharacterBase(const FObjectInitializer& ObjectInitializer);
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	UKOEquipmentComponent* GetEquipmentComponent() const { return EquipmentComponent; }

	UKOMovementSet* GetMovementSet() const { return MovementSet; }
	
	UKOHealthSet* GetHealthSet() const { return HealthSet; }
	
	UKOCombatSet* GetCombatSet() const { return CombatSet; }
	
	UKOGuardSet* GetGuardSet() const { return GuardSet; }
	
protected:
	virtual void BeginPlay() override;

	virtual void PostInitializeComponents() override;

	virtual void InitializeAttributes();

public:
	UFUNCTION()
	virtual void OnCharacterDead(AActor* DeathInstigator);
	
	UFUNCTION()
	void OnMaxWalkSpeedChanged(float OldValue, float NewValue);
	
	UFUNCTION()
	void OnMaxAccelerationChanged(float OldValue, float NewValue);
	
	UFUNCTION()
	void OnBrakingDecelerationChanged(float OldValue, float NewValue);
	
	UFUNCTION()
	void OnGroundFrictionChanged(float OldValue, float NewValue);
	
	UFUNCTION()
	void OnMaxWalkSpeedCrouchedChanged(float OldValue, float NewValue);
	
	UFUNCTION()
	void OnJumpStrengthChanged(float OldValue, float NewValue);
	
	UFUNCTION()
	void OnGravityScaleChanged(float OldValue, float NewValue);
	
	// 세이브 로드
	UFUNCTION()
	void RestoreAliveStateFromLoad();
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<UKOAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atttribute | Combat")
	TObjectPtr<UKOCombatSet> CombatSet;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atttribute | Health")
	TObjectPtr<UKOHealthSet> HealthSet;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atttribute | Movement")
	TObjectPtr<UKOMovementSet> MovementSet;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atttribute | Guard")
	TObjectPtr<UKOGuardSet> GuardSet;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UKOEquipmentComponent> EquipmentComponent;

	// EquipmentComponent로 사용할 실제 클래스. BP_EquipmentComponent 등 자식 Blueprint로 교체 가능.
	// 미지정 시 UKOEquipmentComponent(C++ 기본 클래스)를 사용한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Component")
	TSubclassOf<UKOEquipmentComponent> EquipmentComponentClass;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsDead = false; 
	
};
