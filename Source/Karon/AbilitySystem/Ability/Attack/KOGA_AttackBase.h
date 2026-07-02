#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/KOGameplayAbilityBase.h"
#include "KOGA_AttackBase.generated.h"

class UKOCombatSet;
class UAbilityTask_Tick;

USTRUCT(BlueprintType, Blueprintable)
struct FKODamageEffectData // 데미지 용 
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> EffectClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Level = 1.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float AttackCoefficient = 1.f;
};

USTRUCT(BlueprintType, Blueprintable)
struct FKOEffectData // 추가 효과용 
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> EffectClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Level = 1.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<FGameplayTag, float> SetByCallerValues;
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

USTRUCT(BlueprintType)
struct FKOTraceSocketPair
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName StartSocket = FName("StartTrace");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName EndSocket = FName("EndTrace");
};

USTRUCT(BlueprintType, Blueprintable)
struct FKOTraceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bShowDebug = true;
	
	UPROPERTY()
	UMeshComponent* TraceMesh = nullptr; 
	
	UPROPERTY(EditDefaultsOnly)
	TArray<FKOTraceSocketPair> SocketPairs = {FKOTraceSocketPair{}};
	
	int32 CurrentSocketIndex = 0;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float TraceRadius = 45.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxHitCount = 1; 
	
	UPROPERTY()
	TArray<AActor*> HitActors;
	
	FVector PrevStartLocation = FVector::ZeroVector;
	FVector PrevEndLocation = FVector::ZeroVector;
	
	bool bIsFirstTick = true; 
	
	FName GetStartSocket() const
	{
		if (SocketPairs.IsEmpty()) return FName(""); 
		
		if (SocketPairs.IsValidIndex(CurrentSocketIndex))
			return SocketPairs[CurrentSocketIndex].StartSocket;
		
		return SocketPairs[0].StartSocket; 
	}

	FName GetEndSocket() const
	{
		if (SocketPairs.IsEmpty()) return FName(""); 
		
		if (SocketPairs.IsValidIndex(CurrentSocketIndex))
			return SocketPairs[CurrentSocketIndex].EndSocket;
		
		return SocketPairs[0].EndSocket; 
	}
	
	void SwapSocket()
	{
		if (SocketPairs.IsEmpty()) return;
		CurrentSocketIndex = (CurrentSocketIndex + 1) % SocketPairs.Num();
	}
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
	UFUNCTION()
	virtual void PerformWeaponTrace(float DeltaTime);
	
	UFUNCTION()
	virtual void ResetHitActors();
	
	UMeshComponent* FindTraceMesh(); 
	
	UFUNCTION()
	virtual void OnTargetHit(const FHitResult& Hit);
	
protected:	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace")
	FKOTraceData TraceData; 
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage")
	TArray<FKOAttackMontageData> MontageData;
	
	// 데미지 GE
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	TArray<FKODamageEffectData> DamageEffects;
	
	// 추가 효과 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	TArray<FKOEffectData> AdditionalEffects;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Event")
	FGameplayTagContainer AttackEventTags; 
	
	UAbilityTask_Tick* TickTask;
	
private:
	//자신 효과 핸들
	TArray<FActiveGameplayEffectHandle> SelfEffectsHandles;
};
