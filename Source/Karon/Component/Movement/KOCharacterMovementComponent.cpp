#include "Component/Movement/KOCharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"

UKOCharacterMovementComponent::UKOCharacterMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UKOCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
	
	if (!CharacterOwner) return;
	
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(CharacterOwner);
	if (!ASC) return;
	
	if (MovementMode == MOVE_Falling)
	{
		 ASC->AddLooseGameplayTag(KOGameplayTags::State_Character_Movement_InAir); 
	}
	
	if (PreviousMovementMode == MOVE_Falling && MovementMode != MOVE_Falling)
	{
		 ASC->RemoveLooseGameplayTag(KOGameplayTags::State_Character_Movement_InAir);
	}
}
