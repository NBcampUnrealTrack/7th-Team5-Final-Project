#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/KOGameplayAbilityBase.h"
#include "KOGA_AttackBase.generated.h"

class UKOCombatSet;
class UAbilityTask_Tick;

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

USTRUCT(BlueprintType, Blueprintable)
struct FKOTraceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bShowDebug = true;
	
	UPROPERTY()
	UMeshComponent* TraceMesh = nullptr; 
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName StartSocket = FName("StartTrace");
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName EndSocket = FName("EndTrace");
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float TraceRadius = 45.0f;
	
	UPROPERTY()
	TArray<AActor*> HitActors;
};

UCLASS(Abstract)
class KARON_API UKOGA_AttackBase : public UKOGameplayAbilityBase
{
	GENERATED_BODY()
	
public:
	UKOGA_AttackBase();
	
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;
	
	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility, bool bWasCancelled
	) override;

	virtual void SendAttackEventsToTarget(FGameplayEventData* InEventData);
	
	virtual void SendAttackEventsToTarget(AActor* TargetActor); 
	
	virtual void ApplyHitEffects(FGameplayEventData* InEventData);
	
	virtual void ApplyHitEffects(AActor* TargetActor);
	
	UKOCombatSet* GetCombatSet();

protected:
	UFUNCTION(BlueprintCallable, Category = "Attack|Trace")
	virtual void PerformWeaponTrace(float DeltaTime);
	
	UFUNCTION(BlueprintCallable, Category = "Attack|Trace")
	virtual void ResetHitActors();
	
	UMeshComponent* FindTraceMesh(); 
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace")
	FKOTraceData TraceData; 
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage")
	TArray<FKOAttackMontageData> MontageData;
	
	// 데미지나 추가적인 이팩트 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	TArray<FKOHitEffectData> HitAppliedEffects;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Event")
	FGameplayTagContainer AttackEventTags; 
	
	UAbilityTask_Tick* TickTask;
};
