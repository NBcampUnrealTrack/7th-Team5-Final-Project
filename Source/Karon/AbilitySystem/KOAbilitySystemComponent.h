#pragma once
#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Data/KOGrantSet.h"
#include "KOAbilitySystemComponent.generated.h"

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
	void GiveGrantSet(); 
	
	void ClearGrantSet();
	
protected:
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability | Default")
	TObjectPtr<UKOGrantSet> GrantSet;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability | Default")
	FKOAbilitySetHandles GrantSetHandle; 
};
