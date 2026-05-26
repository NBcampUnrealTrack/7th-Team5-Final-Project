// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "KOBossBase.generated.h"

struct FStreamableHandle;
class UAbilitySystemComponent;
class UKOHealthSet;
class UKOCombatSet;
class UKOMovementSet;
class UKOBossDataAsset;
class UStreamableHandle;

DECLARE_MULTICAST_DELEGATE(FOnBossReady);

UCLASS()
class KARON_API AKOBossBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()
 
public:
	AKOBossBase();
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
 
	// 외부 호출
	UFUNCTION(BlueprintCallable, Category = "Boss")
	void StartAsyncLoad(UKOBossDataAsset* InDataAsset);
 
	// 델리게이트
	FOnBossReady OnBossReady;
 
protected:
	virtual void BeginPlay() override;
	
	virtual void OnBossInitialized() {}
	
	virtual void OnPhaseChanged(int32 NewPhase) {}
	
	virtual void OnGroggyBegin() {}
	virtual void OnGroggyEnd() {}
	
	virtual void OnBossDeath() {}
	
	UPROPERTY(VisibleAnywhere, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
 
	UPROPERTY()
	TObjectPtr<UKOHealthSet> HealthSet;
 
	UPROPERTY()
	TObjectPtr<UKOCombatSet> CombatSet;
 
	UPROPERTY()
	TObjectPtr<UKOMovementSet> MovementSet;
 
	UPROPERTY()
	TObjectPtr<UKOBossDataAsset> DataAsset;
	
	// 비동기 로드 테스트
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="DataTest")
	TObjectPtr<UKOBossDataAsset> DefaultDataAsset;
 
private:
	TSharedPtr<FStreamableHandle> StreamableHandle;
 
	void OnAssetsLoaded();
	void ApplyMeshAndAnim();
	void ApplyStats();
	void ApplyAbilities();
 
	// 페이즈 관리
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float PhaseRatio = 0.5f;
private:
	bool bPhase2Triggered = false;

	// ─── 델리게이트 콜백 함수 ────────────────────────────────
	UFUNCTION()
	void OnHealthChangedCallback(float OldVal, float NewVal);
 
	UFUNCTION()
	void OnMoveSpeedChangedCallback(float OldVal, float NewVal);
};
