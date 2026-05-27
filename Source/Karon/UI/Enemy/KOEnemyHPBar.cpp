// Fill out your copyright notice in the Description page of Project Settings.


#include "KOEnemyHPBar.h"

#include "Character/Enemy/KOBaseEnemy.h"
#include "Components/ProgressBar.h"
#include "Components/WidgetComponent.h"


void UKOEnemyHPBar::NativeConstruct()
{
	Super::NativeConstruct();
	SetVisibility(ESlateVisibility::Hidden);
}

void UKOEnemyHPBar::OnHPChanged(float Percent)
{
	if (GetVisibility()==ESlateVisibility::Hidden)
	{
		SetVisibility(ESlateVisibility::Visible);
	}
	if (HPProgressBar!=nullptr)
	{
		HPProgressBar->SetPercent(Percent);
		if (Percent==0.f)
		{
			SetVisibility(ESlateVisibility::Hidden);
		}
	}
}
