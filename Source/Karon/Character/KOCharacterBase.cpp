#include "KOCharacterBase.h"

#include "GMRouterSubsystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Karon/AbilitySystem/KOAbilitySystemComponent.h"


AKOCharacterBase::AKOCharacterBase(const FObjectInitializer& ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	
}

UAbilitySystemComponent* AKOCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AKOCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
	 BindMovementSet();
}

void AKOCharacterBase::BindMovementSet()
{
	//UGMRouterSubsystem::Subscribe(GetWorld(),Tag,FTypeStruct)
}

void AKOCharacterBase::OnWalkSpeedChanged(float NewWalkSpeed, float OldWalkSpeed)
{
	GetCharacterMovement()->MaxWalkSpeed = NewWalkSpeed;
}

