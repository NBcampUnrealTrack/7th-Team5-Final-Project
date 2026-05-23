#include "KOPreCMCTickComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"


UKOPreCMCTickComponent::UKOPreCMCTickComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}


void UKOPreCMCTickComponent::BeginPlay()
{
	Super::BeginPlay();

	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (!Owner) return;
	
	UCharacterMovementComponent* MovementComponent = Owner->GetCharacterMovement();
	if (!MovementComponent) return;
	
	MovementComponent->PrimaryComponentTick.AddPrerequisite(this, PrimaryComponentTick); 
}


