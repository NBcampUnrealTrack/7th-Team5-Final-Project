// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Enemy/KOEnemyHPBar.h"

#include "Character/Enemy/KOBaseEnemy.h"
#include "Components/Overlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/WidgetComponent.h"


void UKOEnemyHPBar::NativeConstruct()
{
	Super::NativeConstruct();
	HPOverlay->SetVisibility(ESlateVisibility::Hidden);
	BattleOverlay->SetVisibility(ESlateVisibility::Hidden);
}

void UKOEnemyHPBar::HPEffect()
{
	//HP가 보간되는 중 HP가 감소하는 경우
	if (PastFixedPercent!=CurrentProgressPercent)
	{
		CurrentEffectPercent=HPProgressBarBackEffect->GetPercent();
		LerpFloat=0.f;
		PastFixedPercent=CurrentProgressPercent;
	}
	if (LerpFloat>=1.f)
	{
		LerpFloat=0.f;
		GetWorld()->GetTimerManager().ClearTimer(EffectTimerHandle);
		return;
	}
	//HPInterploationTime에 보간이 완료될수 있도록 주기/총시간 만큼 합산.
	LerpFloat+=HPInterpolationInterval/HPInterpolationTime;
	HPProgressBarBackEffect->SetPercent(FMath::Lerp(CurrentEffectPercent,CurrentProgressPercent,LerpFloat));
}

void UKOEnemyHPBar::BattleDamageInit()
{
	AccumulateDamage=0.f;
	DamageTextBlock->SetText(FText::GetEmpty());
}

void UKOEnemyHPBar::CheckIsBattle()
{
	//해당시간뒤에도 전투중이 아니라면 HP바를 숨긴다.
	if (BattleOverlay->GetVisibility()==ESlateVisibility::Hidden)
	{
		HPOverlay->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UKOEnemyHPBar::OnHPChanged(float ProgressPercent,float Damage)
{
	if (HPOverlay->GetVisibility()==ESlateVisibility::Hidden)
	{
		HPOverlay->SetVisibility(ESlateVisibility::Visible);
	}
	if (HPProgressBar!=nullptr)
	{
		HPProgressBar->SetPercent(ProgressPercent);
		CurrentProgressPercent=ProgressPercent;
		if (ProgressPercent==0.f)
		{
			SetVisibility(ESlateVisibility::Hidden);
		}
	}
	if (HPProgressBarBackEffect!=nullptr)
	{
		GetWorld()->GetTimerManager().SetTimer(EffectTimerHandle,this,&UKOEnemyHPBar::HPEffect,HPInterpolationInterval,true);
	}
	if (DamageTextBlock!=nullptr)
	{
		AccumulateDamage+=Damage;
		AccumulateDamage=FMath::RoundToInt(AccumulateDamage);
		DamageTextBlock->SetText(FText::AsNumber(AccumulateDamage));
		GetWorld()->GetTimerManager().SetTimer(BattleTimerHandle,this,&UKOEnemyHPBar::BattleDamageInit,DamageDelayMaxTime,false);
	}
}

void UKOEnemyHPBar::OnBattleChanged(bool bIsBattle)
{
	if (bIsBattle)
	{
		BattleOverlay->SetVisibility(ESlateVisibility::Visible);
		GetWorld()->GetTimerManager().ClearTimer(HPBarTimerHandle);
	}
	else
	{
		BattleOverlay->SetVisibility(ESlateVisibility::Hidden);
		//비전투 상태가 BattleDelayMaxTime보다 오래되면 HP바를 숨긴다.
		GetWorld()->GetTimerManager().SetTimer(HPBarTimerHandle,this,&UKOEnemyHPBar::CheckIsBattle,BattleDelayMaxTime,false);
	}
}
