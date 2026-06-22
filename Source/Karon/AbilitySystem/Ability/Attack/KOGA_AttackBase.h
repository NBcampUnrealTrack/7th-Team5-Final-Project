#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/KOGameplayAbilityBase.h"
#include "KOGA_AttackBase.generated.h"

class UKOCombatSet;

USTRUCT(BlueprintType, Blueprintable)
struct FKOHitEffectData
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect>  EffectClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Level = 0.f; 
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<FGameplayTag, float> SetByCallerValues;
	
	UPROPERTY(BlueprintReadOnly)
	FActiveGameplayEffectHandle Handle; 
};

USTRUCT(BlueprintType, Blueprintable)
struct FKOAttackMontageData
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimMontage* Montage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float PlayRate = 1.f; 
};

UCLASS(Abstract)
class KARON_API UKOGA_AttackBase : public UKOGameplayAbilityBase
{
	GENERATED_BODY()
	
public:
	UKOGA_AttackBase();
	
protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
	
	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility, 
		bool bWasCancelled) override;
	
public:	
	virtual void SendAttackEventsToTarget(FGameplayEventData* InEventData);
	
	virtual void SendAttackEventsToTarget(AActor* TargetActor); 
	
	virtual void ApplyHitEffects(FGameplayEventData* InEventData);
	
	virtual void ApplyHitEffects(AActor* TargetActor);
	
	UKOCombatSet* GetCombatSet();

protected:
	UFUNCTION(BlueprintCallable, Category = "Attack|Trace")
	virtual void PerformWeaponTrace();
	
	UFUNCTION(BlueprintCallable, Category = "Attack|Trace")
	virtual void ClearHitHistory();
	
	UFUNCTION()
	void OnWeaponTraceStarted(FGameplayEventData Payload);
	
	UFUNCTION()
	void OnWeaponTraceEnded(FGameplayEventData Payload);
	
	UPROPERTY()
	FTimerHandle TraceTimerHandle;
	
	UPROPERTY()
	TArray<AActor*> DamagedActors;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Trace")
	FName WeaponStartSocket = FName("StartTrace");
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Trace")
	FName WeaponEndSocket = FName("EndTrace");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Trace")
	float TraceRadius = 45.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage")
	TArray<FKOAttackMontageData> MontageData;
	
	// 데미지나 추가적인 이팩트 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	TArray<FKOHitEffectData> HitAppliedEffects;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Event")
	FGameplayTagContainer AttackEventTags; 

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debug")
	bool bShowDebug = true; 
};
