// Fill out your copyright notice in the Description page of Project Settings.


#include "KO_AnimNotify_SendEventHit.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystem/Tag/Event/KOGameplayTags_Event.h"
#include "Character/Enemy/KOBaseEnemy.h"

UKO_AnimNotify_SendEventHit::UKO_AnimNotify_SendEventHit()
{
	bIsNativeBranchingPoint=true;
}

void UKO_AnimNotify_SendEventHit::BranchingPointNotify(FBranchingPointNotifyPayload& BranchingPointPayload)
{
	Super::BranchingPointNotify(BranchingPointPayload);
	USkeletalMeshComponent* MeshComp = BranchingPointPayload.SkelMeshComponent;
	
	if (AKOCharacterBase* Character=Cast<AKOCharacterBase>(MeshComp->GetOwner()))
	{
		//Event_Hit 태그로 전달
		FGameplayEventData HitGameplayEventData;
		
		if (UAbilitySystemComponent* ASC=Cast<UAbilitySystemComponent>(Character->GetAbilitySystemComponent()))
		{
			ASC->HandleGameplayEvent(KOGameplayTags::Event_Hit, &HitGameplayEventData);
		}
	}
}
