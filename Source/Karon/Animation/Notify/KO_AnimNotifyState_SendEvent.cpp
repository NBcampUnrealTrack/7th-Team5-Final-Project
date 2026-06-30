#include "KO_AnimNotifyState_SendEvent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"

UKO_AnimNotifyState_SendEvent::UKO_AnimNotifyState_SendEvent()
{
	bIsNativeBranchingPoint = true;
}

void UKO_AnimNotifyState_SendEvent::BranchingPointNotifyBegin(FBranchingPointNotifyPayload& BranchingPointPayload)
{
	Super::BranchingPointNotifyBegin(BranchingPointPayload);
	
	USkeletalMeshComponent* MeshComp = BranchingPointPayload.SkelMeshComponent;
	if (!MeshComp) return;
	
	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor) return; 
	
	FGameplayEventData Payload;
	Payload.Instigator = OwnerActor;
	Payload.Target = OwnerActor;
	
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerActor, StartTag, Payload);
}

void UKO_AnimNotifyState_SendEvent::BranchingPointNotifyEnd(FBranchingPointNotifyPayload& BranchingPointPayload)
{
	Super::BranchingPointNotifyEnd(BranchingPointPayload);
	
	USkeletalMeshComponent* MeshComp = BranchingPointPayload.SkelMeshComponent;
	if (!MeshComp) return; 
	
	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor) return; 
	
	FGameplayEventData Payload;
	Payload.Instigator = OwnerActor;
	Payload.Target = OwnerActor;
	
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerActor,EndTag, Payload);
}
