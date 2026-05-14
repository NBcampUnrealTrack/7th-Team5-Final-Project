#pragma once
#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "KOGameplayAbilityBase.generated.h"

UCLASS()
class KARON_API UKOGameplayAbilityBase : public UGameplayAbility
{
	GENERATED_BODY()
	
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
	FActiveGameplayEffectHandle ApplyEffectToTarget(
		AActor* TargetActor,
		TSubclassOf<UGameplayEffect> EffectClass,
		float Level = 1.f
	);
};
