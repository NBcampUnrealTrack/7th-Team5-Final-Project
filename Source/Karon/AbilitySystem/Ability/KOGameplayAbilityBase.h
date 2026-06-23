#pragma once
#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "KOGameplayAbilityBase.generated.h"

UCLASS(Abstract)
class KARON_API UKOGameplayAbilityBase : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	// ─── Ability Life Cycle ───────────────────────────────
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;
	
	virtual void CancelAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateCancelAbility
	) override; 
	
	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled
	) override;
	
public:
	// ─── Actor / Component Accessors ───────────────────────────────
	UFUNCTION(BlueprintPure, Category = "Ability|Context")
	ACharacter* GetAvatarCharacter() const;

	UFUNCTION(BlueprintPure, Category = "Ability|Context")
	AController* GetAvatarController() const;

	UFUNCTION(BlueprintPure, Category = "Ability|Context")
	APlayerState* GetPlayerState() const;

	UFUNCTION(BlueprintPure, Category = "Ability|Context")
	UAbilitySystemComponent* GetASC() const;

	// ─── Tag Utilities ─────────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "Ability|Tags")
	bool HasMatchingTag(FGameplayTag Tag) const;

	UFUNCTION(BlueprintCallable, Category = "Ability|Tags")
	bool HasAllTags(const FGameplayTagContainer& Tags) const;

	UFUNCTION(BlueprintCallable, Category = "Ability|Tags")
	bool HasAnyTags(const FGameplayTagContainer& Tags) const;

	// ─── GE Application ────────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "Ability|Effects")
	FActiveGameplayEffectHandle ApplyEffectToSelf(
		TSubclassOf<UGameplayEffect> EffectClass,
		float Level = 1.f
	);
	
	UFUNCTION(BlueprintCallable, Category = "Ability|Effects")
	FActiveGameplayEffectHandle ApplyEffectSetByCallerToSelf(
		TSubclassOf<UGameplayEffect> EffectClass,
		FGameplayTag DataTag, 
		float Amount,
		float Level = 1.f
	);

	UFUNCTION(BlueprintCallable, Category = "Ability|Effects")
	FActiveGameplayEffectHandle ApplyEffectToTarget(
		AActor* TargetActor,
		TSubclassOf<UGameplayEffect> EffectClass,
		float Level = 1.f
	);
	
	UFUNCTION(BlueprintCallable, Category = "Ability|Effects")
	FActiveGameplayEffectHandle ApplyEffectSetByCallerToTarget(
		AActor* TargetActor,
		TSubclassOf<UGameplayEffect> EffectClass,
		FGameplayTag DataTag, 
		float Amount,
		float Level = 1.f
	);
protected:
	// ─── Cooldown ─────────────────────────────────────────────────────
	virtual UGameplayEffect* GetCooldownGameplayEffect() const override;
	
	virtual const FGameplayTagContainer* GetCooldownTags() const override;
	
	virtual void ApplyCooldown(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const override;
	
	UPROPERTY(EditDefaultsOnly, Category = "Cooldown")
	TSubclassOf<UGameplayEffect> CooldownGEClass;

	// 어빌리티별 쿨타임 식별 태그 (예: Cooldown.Attack.Combo)
	UPROPERTY(EditDefaultsOnly, Category = "Cooldown", Meta = (Categories = "Cooldown"))
	FGameplayTag CooldownTag;

	UPROPERTY(EditDefaultsOnly, Category = "Cooldown")
	float CooldownDuration = 0.f;
	
private:
	mutable FGameplayTagContainer CachedCooldownTags;
};
