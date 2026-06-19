#include "KO_AnimNotifyState_SendEvent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Items/Equipment/KOWeaponBase.h"

UKO_AnimNotifyState_SendEvent::UKO_AnimNotifyState_SendEvent()
{
}

void UKO_AnimNotifyState_SendEvent::BranchingPointNotifyBegin(FBranchingPointNotifyPayload& BranchingPointPayload)
{
	Super::BranchingPointNotifyBegin(BranchingPointPayload);
	
	USkeletalMeshComponent* MeshComp = BranchingPointPayload.SkelMeshComponent;
	if (!MeshComp)
	{
		return;
	}
	
	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor)
	{
		return;
	}
	
	FGameplayEventData Payload;
	Payload.Instigator = OwnerActor;
	Payload.Target = OwnerActor;
	
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerActor,TraceStartTag, Payload);
}

void UKO_AnimNotifyState_SendEvent::BranchingPointNotifyEnd(FBranchingPointNotifyPayload& BranchingPointPayload)
{
	Super::BranchingPointNotifyEnd(BranchingPointPayload);
	
	USkeletalMeshComponent* MeshComp = BranchingPointPayload.SkelMeshComponent;
	if (!MeshComp)
	{
		return;
	}
	
	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor)
	{
		return;
	}
	FGameplayEventData Payload;
	Payload.Instigator = OwnerActor;
	Payload.Target = OwnerActor;
	
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerActor,TraceEndTag, Payload);
}
