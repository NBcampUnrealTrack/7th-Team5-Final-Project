// Fill out your copyright notice in the Description page of Project Settings.


#include "KOGA_AnimNotify_SendEvent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"

void UKOGA_AnimNotify_SendEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                        const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	if (MeshComp && MeshComp->GetOwner())
	{
		FGameplayEventData PayLoad;
		
		PayLoad.EventTag = EventTag;
		
		PayLoad.Instigator = MeshComp->GetOwner();
		PayLoad.Target = MeshComp->GetOwner();
		
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp->GetOwner(), EventTag, PayLoad);
	}
}
