#include "KO_AnimNotify_SendGameplayEvent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

UKO_AnimNotify_SendGameplayEvent::UKO_AnimNotify_SendGameplayEvent()
{
}

void UKO_AnimNotify_SendGameplayEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	if (!MeshComp)
	{
		return;
	}
	
	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor)
	{
		return;
	}
	
	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwnerActor);
	if (ASC)
	{
		FGameplayEventData PayloadData;
		PayloadData.EventTag = EventTag;
		PayloadData.Instigator = OwnerActor;
		PayloadData.Target = OwnerActor;
		
		ASC->HandleGameplayEvent(EventTag, &PayloadData);
	}
}

FString UKO_AnimNotify_SendGameplayEvent::GetNotifyName_Implementation() const
{
	if (EventTag.IsValid())
	{
		return EventTag.ToString();
	}
	
	return Super::GetNotifyName_Implementation();
}
