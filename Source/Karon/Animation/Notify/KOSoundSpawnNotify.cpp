// Fill out your copyright notice in the Description page of Project Settings.


#include "KOSoundSpawnNotify.h"

#include "Kismet/GameplayStatics.h"

UKOSoundSpawnNotify::UKOSoundSpawnNotify()
{
	bIsNativeBranchingPoint=true;
}

void UKOSoundSpawnNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!SoundBase)
	{
		return;
	}
	
	if (MeshComp)
	{
		UWorld* World = MeshComp->GetWorld();
		if (World&&MeshComp->GetOwner())
		{
			UGameplayStatics::PlaySoundAtLocation(
				World, 
				SoundBase, 
				MeshComp->GetOwner()->GetActorLocation(), 
				FRotator::ZeroRotator, 
				Volume,              
				PlayRate,                  
				StartTime                
			);
		}
	}
}

