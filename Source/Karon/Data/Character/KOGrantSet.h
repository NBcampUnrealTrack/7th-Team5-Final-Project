#pragma once
#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
#include "KOGrantSet.generated.h"

class UGameplayAbility; 
class UAbilitySystemComponent;
class UGameplayEffect; 

USTRUCT(BlueprintType)
struct FKOActiveAbilityEntry
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
struct FKOSubAbilityEntry
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayAbility> Ability = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 AbilityLevel = 1;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bStartActivated = false;
};

USTRUCT(BlueprintType)
struct FKOEffectEntry
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> Effect = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float EffectLevel = 1.f;
};

USTRUCT(BlueprintType)
struct FKOAbilitySetHandles
{
	GENERATED_BODY()
public:
	TArray<FGameplayAbilitySpecHandle> AbilityHandles;
	TArray<FActiveGameplayEffectHandle> EffectHandles;

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
	TArray<FKOActiveAbilityEntry> ActiveAbilities;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "Ability"))
	TArray<FKOSubAbilityEntry> SubAbilities;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "Effect"))
	TArray<FKOEffectEntry> GrantedEffects;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
	TSubclassOf<UGameplayEffect> AttributeInitializationEffect;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "Attribute"))
	FDataTableRowHandle StatRow;
};
