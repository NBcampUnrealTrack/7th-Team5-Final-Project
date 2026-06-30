#include "KOPlayerState.h"

#include "AbilitySystem/KOAbilitySystemComponent.h"
#include "AbilitySystem/Attribute/KOCombatSet.h"
#include "AbilitySystem/Attribute/KOGuardSet.h"
#include "AbilitySystem/Attribute/KOHealthSet.h"
#include "AbilitySystem/Attribute/KOMovementSet.h"
#include "AbilitySystem/Attribute/KOStaminaSet.h"

AKOPlayerState::AKOPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UKOAbilitySystemComponent>(TEXT("ASC"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed); 
	
	HealthSet   = CreateDefaultSubobject<UKOHealthSet>(TEXT("HealthSet"));
	StaminaSet  = CreateDefaultSubobject<UKOStaminaSet>(TEXT("StaminaSet"));
	CombatSet   = CreateDefaultSubobject<UKOCombatSet>(TEXT("CombatSet"));
	MovementSet = CreateDefaultSubobject<UKOMovementSet>(TEXT("MovementSet"));
	GuardSet = CreateDefaultSubobject<UKOGuardSet>(TEXT("GuardSet"));
}

UAbilitySystemComponent* AKOPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AKOPlayerState::BeginPlay()
{
	Super::BeginPlay();
}

