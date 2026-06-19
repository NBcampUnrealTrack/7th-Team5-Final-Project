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
	
public:	
	virtual void SendAttackEventsToTarget(FGameplayEventData* InEventData);
	
	virtual void ApplyHitEffects(FGameplayEventData* InEventData);
	
	UKOCombatSet* GetCombatSet();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage")
	TArray<FKOAttackMontageData> MontageDatas;
	
	// 데미지나 추가적인 이팩트 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	TArray<FKOHitEffectData> HitAppliedEffects;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Event")
	FGameplayTagContainer AttackEventTags; 

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debug")
	bool bShowDebug = true; 
};
