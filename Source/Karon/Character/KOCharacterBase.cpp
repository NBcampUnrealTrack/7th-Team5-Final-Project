#include "KOCharacterBase.h"

#include "KHS_GMRouterManager.h"
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
	
	// BindMovementSet();
}

void AKOCharacterBase::BindMovementSet()
{
	if (UKHS_GMRouterManager* MessageSubsystem = GetGameInstance()->GetSubsystem<UKHS_GMRouterManager>())
	{
		// MessageSubsystem->SubscribeToMessage(MessageSubsystem, OnWalkSpeedChanged);
	}
}

void AKOCharacterBase::OnWalkSpeedChanged(float NewWalkSpeed, float OldWalkSpeed)
{
	GetCharacterMovement()->MaxWalkSpeed = NewWalkSpeed;
}

