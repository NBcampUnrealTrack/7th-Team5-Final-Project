// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UI/KOActivatableWidget.h"
#include "KOInGameHUD.generated.h"

class AKOHeroCharacter;
class UKOHealthSet;
class UKOStaminaSet;
class UProgressBar;
/**
 * 가장 하단에 깔려서 보여질 HUD
 */
UCLASS()
class KARON_API UKOInGameHUD : public UKOActivatableWidget
{
	GENERATED_BODY()
	
	
public:
	UFUNCTION(BlueprintCallable, Category = "HUD|KeyGuide")
	void SetBuildKeyGuideMode(bool bBuildMode);
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UProgressBar> HealthBar;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UProgressBar> StaminaBar;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> NormalKeyGuide;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> BuildKeyGuide;
	
private:
	float CachedCurrentHealth  = 0.f;
	float CachedMaxHealth      = 0.f;
	float CachedCurrentStamina = 0.f;
	float CachedMaxStamina     = 0.f;
	
	UPROPERTY()
	TObjectPtr<UKOHealthSet>  CachedHealthSet;
	UPROPERTY()
	TObjectPtr<UKOStaminaSet> CachedStaminaSet;
	
	UFUNCTION()
	void OnCurrentHealthChanged(float OldValue, float NewValue);
	
	UFUNCTION()
	void OnMaxHealthChanged(float OldValue, float NewValue);
	
	UFUNCTION()
	void OnCurrentStaminaChanged(float OldValue, float NewValue);
	
	UFUNCTION()
	void OnMaxStaminaChanged(float OldValue, float NewValue);
	
	void RefreshHealthBar();
	void RefreshStaminaBar();
};
