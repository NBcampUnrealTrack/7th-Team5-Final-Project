#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "KOInputConfig.generated.h"

class UInputAction; 

USTRUCT(BlueprintType)
struct FKOInputAction
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<const UInputAction> InputAction = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Category = "InputTag"))
	FGameplayTag InputTag; 
};

UCLASS(BlueprintType, Const)
class KARON_API UKOInputConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UKOInputConfig(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable)
	const UInputAction* FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

	UFUNCTION(BlueprintCallable)
	const UInputAction* FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
	TArray<FKOInputAction> NativeInputActions;
 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
	TArray<FKOInputAction> AbilityInputActions;
};
