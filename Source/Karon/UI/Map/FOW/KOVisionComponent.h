#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "KOVisionComponent.generated.h"

class AKOFogManager;
 
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), DisplayName="KOFogVision")
class KARON_API UKOVisionComponent : public UActorComponent
{
	GENERATED_BODY()
 
public:
	UKOVisionComponent();
 
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
 
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vision")
	float VisionRadius = 1000.f;
 
	FVector GetVisionLocation() const;
 
private:
	TWeakObjectPtr<AKOFogManager> CachedFogManager;
};
 
