// Fill out your copyright notice in the Description page of Project Settings.


#include "KOEnemyBaseUI.h"

void UKOEnemyBaseUI::OnVisibilityChanged(bool bIsVisible)
{
	if (bIsVisible)
	{
		SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		SetVisibility(ESlateVisibility::Hidden);
	}
}

void UKOEnemyBaseUI::NativeConstruct()
{
	Super::NativeConstruct();
	
	SetVisibility(ESlateVisibility::Hidden);
}
