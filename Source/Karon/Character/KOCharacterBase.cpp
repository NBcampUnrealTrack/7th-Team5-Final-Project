#include "KOCharacterBase.h"
#include "AbilitySystem/Attribute/KOHealthSet.h"
#include "AbilitySystem/Attribute/KOMovementSet.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Component/KOCharacterMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Karon/AbilitySystem/KOAbilitySystemComponent.h"


AKOCharacterBase::AKOCharacterBase(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer.SetDefaultSubobjectClass<UKOCharacterMovementComponent>(
		ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
	
	HealthSet = CreateDefaultSubobject<UKOHealthSet>(FName("HealthSet"));
	MovementSet = CreateDefaultSubobject<UKOMovementSet>(FName("MovementSet"));
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
	MovementSet->OnMoveSpeedChanged.AddDynamic(this, &ThisClass::OnMoveSpeedChanged);
	OnMoveSpeedChanged(600.f, MovementSet->GetMoveSpeed());
	
	MovementSet->OnJumpStrengthChanged.AddDynamic(this, &ThisClass::OnJumpStrengthChanged);
	OnJumpStrengthChanged(420, MovementSet->GetJumpStrength());
}

void AKOCharacterBase::OnMoveSpeedChanged(float OldWalkSpeed, float NewWalkSpeed)
{
	GetCharacterMovement()->MaxWalkSpeed = NewWalkSpeed;
}

void AKOCharacterBase::OnJumpStrengthChanged(float OldJumpStrength, float NewJumpStrength)
{
	GetCharacterMovement()->JumpZVelocity = NewJumpStrength;
}

