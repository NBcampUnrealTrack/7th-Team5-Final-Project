#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "KOAbilitySet.generated.h"

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

UCLASS(BlueprintType, Const)
class KARON_API UKOAbilitySet : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UKOAbilitySet(const FObjectInitializer& ObjectInitializer);
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void GiveAbilities(UAbilitySystemComponent* ASC) const;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "Ability"))
	TArray<FKOAbilityEntry> GrantedAbilities;
};
