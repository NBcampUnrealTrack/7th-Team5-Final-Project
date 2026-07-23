#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "KO_HitData.generated.h"

USTRUCT(BlueprintType)
struct FKOHitData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitData")
	float HitStopDuration = 0.08f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitData")
	float HitStopTimeDilation = 0.01f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitData")
	float KnockBackAmount = 1000.0f;
};

UCLASS(EditInlineNew, DefaultToInstanced, Blueprintable)
class KARON_API UKO_HitData : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitData")
	FKOHitData HitData;
};
