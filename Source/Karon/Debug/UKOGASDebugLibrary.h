#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "UKOGASDebugLibrary.generated.h"


USTRUCT(BlueprintType)
struct FKOAttributeInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly) 
	FString AttributeSetName;
	
	UPROPERTY(BlueprintReadOnly)
	FString AttributeName;
	
	UPROPERTY(BlueprintReadOnly) 
	float BaseValue = 0.f;
	
	UPROPERTY(BlueprintReadOnly)
	float CurrentValue = 0.f;
};



UCLASS()
class KARON_API UUKOGASDebugLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	// ─── GameplayEvent ──────────────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category="KO|Debug|GAS", meta=(ToolTip="선택한 액터에게 GameplayEvent 를 전송합니다"))
	static void SendDebugEvent(AActor* TargetActor, FGameplayTag EventTag, float Magnitude = 0.f);

	// ─── GameplayEffect ─────────────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category="KO|Debug|GAS", meta=(ToolTip="선택한 액터에게 GameplayEffect 를 적용합니다"))
	static FActiveGameplayEffectHandle ApplyDebugGE(AActor* TargetActor, TSubclassOf<UGameplayEffect> GEClass);
	
	UFUNCTION(BlueprintCallable, Category="KO|Debug|GAS")
	static void RemoveDebugGE(AActor* TargetActor, FActiveGameplayEffectHandle Handle);

	UFUNCTION(BlueprintCallable, Category="KO|Debug|GAS")
	static TArray<FString> GetActiveGENames(AActor* TargetActor);
	
	// ─── GameplayAbility ────────────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category="KO|Debug|GAS", meta=(ToolTip="선택한 액터에게 GameplayAbility 를 부여합니다"))
	static FGameplayAbilitySpecHandle GrantDebugAbility(AActor* TargetActor, TSubclassOf<UGameplayAbility> GAClass, int32 Level = 1);
	
	UFUNCTION(BlueprintCallable, Category="KO|Debug|GAS", meta=(ToolTip="선택한 액터의 GameplayAbility 를 제거합니다"))
	static void RemoveDebugAbility(AActor* TargetActor, FGameplayAbilitySpecHandle Handle);

	UFUNCTION(BlueprintCallable, Category="KO|Debug|GAS")
	static TArray<FString> GetActiveAbilityNames(AActor* TargetActor);
	
	// ─── GameplayTag ────────────────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category="KO|Debug|GAS")
	static FGameplayTagContainer GetCurrentTags(AActor* TargetActor);

	UFUNCTION(BlueprintCallable, Category="KO|Debug|GAS", meta=(ToolTip="Loose 태그를 추가합니다 (GE 없이 직접 부착)"))
	static void AddLooseTag(AActor* TargetActor, FGameplayTag Tag);

	UFUNCTION(BlueprintCallable, Category="KO|Debug|GAS")
	static void RemoveLooseTag(AActor* TargetActor, FGameplayTag Tag);
	
	// ─── Attributes  ────────────────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category="KO|Debug|GAS")
	static TArray<FKOAttributeInfo> GetAllAttributes(AActor* TargetActor);
	
	UFUNCTION(BlueprintCallable, Category="KO|Debug|GAS")
	static void SetAttributeBaseValue(AActor* TargetActor, const FString& AttributeName, float NewValue);
};


