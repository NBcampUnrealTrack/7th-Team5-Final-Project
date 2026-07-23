#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Actor.h"
#include "AttributeSet.h"
#include "KOGC_OverClock_Aura.generated.h"

class UMaterialInterface;
class UMaterialInstanceDynamic;
class UPostProcessComponent;

UCLASS()
class KARON_API AKOGC_OverClock_Aura : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()

public:
	AKOGC_OverClock_Aura();

protected:
	virtual void Tick(float DeltaTime) override;
	
	virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	virtual bool OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

	UPROPERTY(EditAnywhere, Category = "TimeDilation")
	float SlowMotionScale;

	UPROPERTY(EditAnywhere, Category = "TimeDilation")
	float SlowMotionRealTimeDuration;
	
private:
	UPROPERTY(VisibleAnywhere, Category = "PostProcess")
	TObjectPtr<UPostProcessComponent> PostProcessComponent;

	UPROPERTY(EditAnywhere, Category = "PostProcess")
	TObjectPtr<UMaterialInterface> VignetteMaterialSource;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> OverclockMID;

	float CurrentAlpha;
	float TargetAlpha;
	
	bool bIsFadingOut;
	
	FTimerHandle TimeDilationTimerHandle;

	void RestoreTimeDilation();
};
