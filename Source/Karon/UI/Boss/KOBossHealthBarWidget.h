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
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> BossHealth;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> BossName;
	
private:
	UPROPERTY()
	TObjectPtr<AKOBossBase> BossRef;
	
	UFUNCTION()
	void OnHealthChanged(float OldVal, float NewVal);
	
	void UpdateHealthBar(float Current, float Max);
};
