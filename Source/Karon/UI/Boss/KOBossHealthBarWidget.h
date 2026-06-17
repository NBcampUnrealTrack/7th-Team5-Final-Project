#pragma once

#include "CoreMinimal.h"
#include "UI/KOActivatableWidget.h"
#include "KOBossHealthBarWidget.generated.h"

class AKOBossBase;
class UTextBlock;
class UProgressBar;

UCLASS()
class KARON_API UKOBossHealthBarWidget : public UKOActivatableWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category="Boss|UI")
	void SetBoss(AKOBossBase* InBoss);
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> BossHealth;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> BossHealthYellow;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> BossName;
	
	// 노란 체력바 지연 시간
	UPROPERTY(EditAnywhere, Category = "HealthBar")
	float YellowDelayTime = 1.5f;

	// 노란 체력바 감소 속도
	UPROPERTY(EditAnywhere, Category = "HealthBar")
	float YellowDecreaseSpeed = 0.8f;
	
private:
	UFUNCTION()
	void OnHealthChanged(float OldVal, float NewVal);
	void UpdateHealthBar(float Current, float Max);
	
	UFUNCTION()
	void OnBossDetected(AKOBossBase* DetectedBoss);
	
	UFUNCTION()
	void OnBossDiedCallback();
	
	void OnFadeOutFinished();
	
	UPROPERTY()
	TObjectPtr<AKOBossBase> BossRef;
	
	float MaxHP = 0.f;
	float CurrentHP = 0.f;
	float YellowHP = 0.f;
	
	// UI Param
	float TimeSinceLastHit = 0.f;
	bool bYellowDecreasing = false;
	
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> FadeIn;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> FadeOut;
	
	FTimerHandle FadeOutTimerHandle;
	
};
