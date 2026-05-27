#include "Character/Enemy/Boss/Anim/KOAN_BossShockWave.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"

void UKOAN_BossShockWave::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp || !MeshComp->GetOwner())
	{
		return;
	}

	IAbilitySystemInterface* ASCInterface = 
		Cast<IAbilitySystemInterface>(MeshComp->GetOwner());
	if (!ASCInterface)
	{
		return;
	}

	UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	FGameplayEventData EventData;
	ASC->HandleGameplayEvent(
		KOGameplayTags::Event_Boss01_Shockwave,
		&EventData
	);
}
