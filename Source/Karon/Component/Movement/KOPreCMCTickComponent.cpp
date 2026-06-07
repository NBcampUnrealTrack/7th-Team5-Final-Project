#include "Component/Movement/KOPreCMCTickComponent.h"

#include "AbilitySystem/Attribute/KOMovementSet.h"
#include "Character/KOCharacterBase.h"
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

	CachedOwner = Cast<AKOCharacterBase>(GetOwner());
	if (!CachedOwner) return;
	
	CachedAbilitySystemComponent = CachedOwner->GetAbilitySystemComponent();
	
	CachedMovementComponent = CachedOwner->GetCharacterMovement();
	if (!CachedMovementComponent) return;
	
	CachedMovementComponent->PrimaryComponentTick.AddPrerequisite(this, PrimaryComponentTick); 
	
	CachedMovementSet = CachedOwner->GetMovementSet();
}

void UKOPreCMCTickComponent::TickComponent(
	float DeltaTime, 
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	UpdateRotation();
	
	UpdateMovement();
}

void UKOPreCMCTickComponent::UpdateRotation()
{
	if (!CachedMovementComponent) return;
	
	CachedMovementComponent->RotationRate = CachedMovementComponent->IsFalling() ? 
		FRotator(0.0f, 0.0f, 200.0f) :
		FRotator(0.0f, 0.0f, -1.0f);
}

void UKOPreCMCTickComponent::UpdateMovement()
{
	CachedMovementComponent->MaxWalkSpeed = CachedMovementSet->GetMaxWalkSpeed();
	CachedMovementComponent->MaxAcceleration = CachedMovementSet->GetMaxAcceleration();
	CachedMovementComponent->BrakingDecelerationWalking = CachedMovementSet->GetBrakingDecelerationWalking();
	CachedMovementComponent->GroundFriction = CachedMovementSet->GetGroundFriction();
	CachedMovementComponent->MaxWalkSpeedCrouched = CachedMovementSet->GetMaxWalkSpeedCrouched();
}


