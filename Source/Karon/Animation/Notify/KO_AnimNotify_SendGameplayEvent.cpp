#include "KO_AnimNotify_SendGameplayEvent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystemBlueprintLibrary.h"

UKO_AnimNotify_SendGameplayEvent::UKO_AnimNotify_SendGameplayEvent()
{
}

void UKO_AnimNotify_SendGameplayEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;
	
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
	if (!ASC) return;
	
	FGameplayEventData PayLoad;
	
	ASC->HandleGameplayEvent(EventTag, &PayLoad);
}

FString UKO_AnimNotify_SendGameplayEvent::GetNotifyName_Implementation() const
{
	if (EventTag.IsValid())
	{
		return EventTag.ToString();
	}
	
	return Super::GetNotifyName_Implementation();
}
