#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "KOGrantSet.generated.h"

class UGameplayAbility; 
class UAbilitySystemComponent;

USTRUCT(BlueprintType)
struct FKOAbilityEntry
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayAbility> Ability = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 AbilityLevel = 1;
};

USTRUCT(BlueprintType)
struct FKOEffectEntry
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<class UGameplayEffect> Effect = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float EffectLevel = 1.f;
};

USTRUCT(BlueprintType)
struct FKOAbilitySetHandles
{
	GENERATED_BODY()
public:
	TArray<struct FGameplayAbilitySpecHandle>  AbilityHandles;
	TArray<struct FActiveGameplayEffectHandle> EffectHandles;

	void RemoveFromASC(UAbilitySystemComponent* ASC);
};

UCLASS(BlueprintType, Const)
class KARON_API UKOGrantSet : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UKOGrantSet(const FObjectInitializer& ObjectInitializer);
	
	UFUNCTION(BlueprintAuthorityOnly)
	void GiveToAsc(UAbilitySystemComponent* ASC, FKOAbilitySetHandles& OutHandles);

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "Ability"))
	TArray<FKOAbilityEntry> GrantedAbilities;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "Effect"))
	TArray<FKOEffectEntry> GrantedEffects;
};
