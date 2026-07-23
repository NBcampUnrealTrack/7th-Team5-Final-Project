// Fill out your copyright notice in the Description page of Project Settings.


#include "KO_OutFitController.h"
#include "UI/Loading/KOLoadingUiSubsystem.h"


void AKO_OutFitController::BeginPlay()
{
	Super::BeginPlay();
	
	
	if (auto* LoadingSubsystem = GetGameInstance()->GetSubsystem<UKOLoadingUiSubsystem>())
	{
		LoadingSubsystem->HideLoadingScreen();
	}
}
