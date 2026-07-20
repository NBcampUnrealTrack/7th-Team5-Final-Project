// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UI/KOActivatableWidget.h"
#include "Data/Type/KOSkillTypes.h"
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
	virtual void NativeDestruct()  override;

	/** 스킬 퀵슬롯 배정이 바뀔 때 호출. Blueprint에서 오버라이드해 HUD를 갱신한다. */
	UFUNCTION(BlueprintNativeEvent, Category = "HUD|SkillQuickSlot")
	void BP_OnSkillQuickSlotChanged(ESkillQuickSlotKey SlotKey, FName SkillName);
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UProgressBar> HealthBar;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UProgressBar> StaminaBar;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> NormalKeyGuide;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> BuildKeyGuide;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> WBP_ClockGauge;
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
	
	UFUNCTION()
	void OnStaminaExhausted(bool bStaminaExhausted);
	
	void RefreshHealthBar();
	void RefreshStaminaBar();

	FGameplayMessageHandle       SkillQuickSlotChangedHandle;
	FGameplayMessageCallback     SkillQuickSlotChangedCallback;

	UFUNCTION()
	void HandleSkillQuickSlotChangedMessage(FGameplayTag Channel, const FInstancedStruct& Payload);
};
