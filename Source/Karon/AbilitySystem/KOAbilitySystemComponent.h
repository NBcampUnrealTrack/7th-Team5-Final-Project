#pragma once
#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "KOAbilitySystemComponent.generated.h"

class UKOAbilitySet; 

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class KARON_API UKOAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
public:
	void AbilityInputTagPressed(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);
	
	void ProcessAbilityInput(float DeltaTime, bool bGamePaused);
	
	void ClearAbilityInput();
	
public:
	void GiveDefaultAbilities(); 
	
protected:
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability | Default")
	UKOAbilitySet* DefaultAbilitySet;
	
};
