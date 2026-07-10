#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Actor.h"
#include "KOGC_HitImpact.generated.h"

UCLASS()
class KARON_API AKOGC_HitImpact : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()
public:
	AKOGC_HitImpact();

protected:
	virtual void Tick(float DeltaTime) override;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual bool OnExecute_Implementation(
		AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flash")
	TObjectPtr<UMaterialInterface> OverlayMaterialBase;
	
	UPROPERTY(EditDefaultsOnly, Category = "Flash")
	FName FlashParamName = TEXT("HitFlashAmount");

	UPROPERTY(EditDefaultsOnly, Category = "Flash")
	float FlashDuration = 0.15f;

	UPROPERTY(EditDefaultsOnly, Category = "Shake")
	float ShakeAmplitude = 2.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Shake")
	float ShakeFrequency = 40.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Shake")
	float ShakeDuration = 0.15f;
	
private:
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> FlashMID;

	TArray<TWeakObjectPtr<USkeletalMeshComponent>> AffectedMeshes;
	TMap<TWeakObjectPtr<USkeletalMeshComponent>, FVector> OriginalRelativeLocations;
	
	float ElapsedTime = 0.f;
	bool bFlashing = false;
};
