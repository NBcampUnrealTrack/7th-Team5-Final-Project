#pragma once

#if WITH_EDITOR

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "UKOGASDebugLibrary.generated.h"

UCLASS()
class KARON_API UUKOGASDebugLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	// 태그로 GameplayEvent 발생
	UFUNCTION(BlueprintCallable, Category="KO|Debug|GAS", meta=(ToolTip="선택한 액터에게 GameplayEvent 를 전송합니다"))
	static void SendDebugEvent(AActor* TargetActor, FGameplayTag EventTag, float Magnitude = 0.f);

	// GE 적용 후 핸들 반환 (제거 시 필요)
	UFUNCTION(BlueprintCallable, Category="KO|Debug|GAS", meta=(ToolTip="선택한 액터에게 GameplayEffect 를 적용합니다"))
	static FActiveGameplayEffectHandle ApplyDebugGE(AActor* TargetActor, TSubclassOf<UGameplayEffect> GEClass);

	// 핸들로 GE 제거
	UFUNCTION(BlueprintCallable, Category="KO|Debug|GAS")
	static void RemoveDebugGE(AActor* TargetActor, FActiveGameplayEffectHandle Handle);

	// 현재 보유 태그 반환
	UFUNCTION(BlueprintCallable, Category="KO|Debug|GAS")
	static FGameplayTagContainer GetCurrentTags(AActor* TargetActor);

	// 활성 GE 이름 목록 반환 (표시용)
	UFUNCTION(BlueprintCallable, Category="KO|Debug|GAS")
	static TArray<FString> GetActiveGENames(AActor* TargetActor);
};

#endif
