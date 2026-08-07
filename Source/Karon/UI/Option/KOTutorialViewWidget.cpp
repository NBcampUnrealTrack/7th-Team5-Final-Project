// Fill out your copyright notice in the Description page of Project Settings.


#include "KOTutorialViewWidget.h"

#include "Components/Button.h"
#include "Subsystem/KOTutorialSubsystem.h"

void UKOTutorialViewWidget::PlayTutorial()
{
	if (TutorialName.Compare("None")==0)
	{
		return;
	}
	if (UKOTutorialSubsystem* TutorialSubsystem = UKOTutorialSubsystem::Get(this))
	{
		TutorialSubsystem->Preload(this,TutorialName);
	}
}

void UKOTutorialViewWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	Button_Play->IsFocusable = false;
	Button_Play->OnClicked.AddDynamic(this, &ThisClass::PlayTutorial);
}
