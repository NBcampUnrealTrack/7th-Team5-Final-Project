// Fill out your copyright notice in the Description page of Project Settings.


#include "KOTeleportActor.h"

#include "Kismet/GameplayStatics.h"


// Sets default values
AKOTeleportActor::AKOTeleportActor()
{
	
}

bool AKOTeleportActor::CanInteract(AActor* Interactor) const
{
	return true;
}

void AKOTeleportActor::OnInteract(AActor* Interactor)
{
	if (IsValid(TargetTeleportActor))
	{
		APawn* Player=UGameplayStatics::GetPlayerPawn(this, 0);
		if (Player)
		{
			Player->SetActorLocation(TargetTeleportActor->GetActorLocation());
		}
	}
	else
	{
		return;
	}
}

FText AKOTeleportActor::GetInteractionPrompt() const
{
	return NSLOCTEXT(
			 "KOTeleport",
			 "Move",
			 "이동하기"
		  );
}

