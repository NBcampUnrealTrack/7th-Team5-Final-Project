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

float AKOPlayerState::GetHealthForSave() const
{
	const UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		return 0.f;
	}

	return ASC->GetNumericAttribute(UKOHealthSet::GetHealthAttribute());
}

void AKOPlayerState::LoadHealthFromSave(float SavedHealth)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	const float MaxHealth = ASC->GetNumericAttribute(UKOHealthSet::GetMaxHealthAttribute());

	const float ClampedHealth = FMath::Clamp(SavedHealth, 0.f, MaxHealth);

	ASC->SetNumericAttributeBase(UKOHealthSet::GetHealthAttribute(), ClampedHealth);
}

void AKOPlayerState::BeginPlay()
{
	Super::BeginPlay();
}

