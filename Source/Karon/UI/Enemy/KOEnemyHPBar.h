// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "KOEnemyHPBar.generated.h"

class UOverlay;
class UTextBlock;
class UProgressBar;
/**
 * 
 */
UCLASS()
class KARON_API UKOEnemyHPBar : public UCommonUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION()
	void OnHPChanged(float ProgressPercent,float Damage);
	
	UFUNCTION()
	void OnBattleChanged(bool bIsBattle);
protected:
	virtual void NativeConstruct() override;	
private:
	void HPEffect();
	void BattleDamageInit();
	void CheckIsBattle();
	

private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UProgressBar> HPProgressBar;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UProgressBar> HPProgressBarBackEffect;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> DamageTextBlock;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UOverlay> HPOverlay;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UOverlay> BattleOverlay;
	
	UPROPERTY()
	FTimerHandle EffectTimerHandle;
	UPROPERTY()
	FTimerHandle BattleTimerHandle;
	UPROPERTY()
	FTimerHandle HPBarTimerHandle;
	
	//데미지 텍스트에서 누적되도록 표시되는 최대시간
	float DamageDelayMaxTime=2.f;
	
	float AccumulateDamage=0.f;
	
	//피격시 BackEffect가 현재 HP 퍼센트 만큼 보간되어 도달할 시간
	float HPInterpolationTime=1.f;
	
	//BackEffect 갱신 간격
	float HPInterpolationInterval=0.05f;
	
	float LerpFloat=0.f;
	float CurrentProgressPercent=1.f;
	float CurrentEffectPercent=1.f;
	float PastFixedPercent=1.f;
	
	//데미지를 안받는 동안 HP바 최대 표시 시간
	float BattleDelayMaxTime=5.f;
};
