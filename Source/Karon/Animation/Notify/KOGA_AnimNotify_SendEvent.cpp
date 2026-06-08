#include "KOGA_AnimNotify_SendEvent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTypes.h"

void UKOGA_AnimNotify_SendEvent::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;
	
	// 이렇게 해야 애니메이션 쪽 열어도 에러 코드 안남. 
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
	if (!ASC) return;
	
	FGameplayEventData PayLoad;
	
	ASC->HandleGameplayEvent(EventTag, &PayLoad);
}
