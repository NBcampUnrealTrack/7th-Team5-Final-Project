#include "KOCharacterBase.h"

#include "AbilitySystem/Attribute/KOMovementSet.h"
#include "Component/Inventory/KOEquipmentComponent.h"
#include "Component/Movement/KOCharacterMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Karon/AbilitySystem/KOAbilitySystemComponent.h"
#include "Utility/Log/KOLogManager.h"


AKOCharacterBase::AKOCharacterBase(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer.SetDefaultSubobjectClass<UKOCharacterMovementComponent>(
		ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
	
	EquipmentComponent = CreateDefaultSubobject<UKOEquipmentComponent>(TEXT("EquipmentComponent"));
}

UAbilitySystemComponent* AKOCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AKOCharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

void AKOCharacterBase::InitializeAttributes()
{
	if (MovementSet)
	{
		MovementSet->OnMaxWalkSpeedBaseChanged.AddDynamic(this, &ThisClass::OnMaxWalkSpeedChanged);
		MovementSet->OnMaxAccelerationChanged.AddDynamic(this, &ThisClass::OnMaxAccelerationChanged);
		MovementSet->OnBrakingDecelerationWalkingChanged.AddDynamic(this, &ThisClass::OnBrakingDecelerationChanged);
		MovementSet->OnGroundFrictionChanged.AddDynamic(this, &ThisClass::OnGroundFrictionChanged);
		MovementSet->OnMaxWalkSpeedCrouchedChanged.AddDynamic(this, &ThisClass::OnMaxWalkSpeedChanged);
		MovementSet->OnJumpStrengthChanged.AddDynamic(this, &ThisClass::OnJumpStrengthChanged);
	}
}

void AKOCharacterBase::OnCharacterDead(AActor* DeathInstigator)
{
	KO_LOG(Combat, Warning, TEXT("[%s] is Dead.      Instigator : [%s]"),
		*GetName(),
		DeathInstigator ? *DeathInstigator->GetName() : TEXT("Unknown")
	);
	
	if (UCharacterMovementComponent* CMC = GetCharacterMovement())
	{
		CMC->DisableMovement();
		CMC->StopMovementImmediately(); 
	}
	
	if (GetMesh())
	{
		GetMesh()->bPauseAnims = true;
	}
}

void AKOCharacterBase::OnMaxWalkSpeedChanged(float OldValue, float NewValue)
{
	GetCharacterMovement()->MaxWalkSpeed = NewValue;
}

void AKOCharacterBase::OnMaxAccelerationChanged(float OldValue, float NewValue)
{
	GetCharacterMovement()->MaxAcceleration = NewValue;
}

void AKOCharacterBase::OnBrakingDecelerationChanged(float OldValue, float NewValue)
{
	GetCharacterMovement()->BrakingDecelerationWalking = NewValue;
}

void AKOCharacterBase::OnGroundFrictionChanged(float OldValue, float NewValue)
{
	GetCharacterMovement()->GroundFriction = NewValue;
}

void AKOCharacterBase::OnMaxWalkSpeedCrouchedChanged(float OldValue, float NewValue)
{
	GetCharacterMovement()->MaxWalkSpeedCrouched = NewValue;
}

void AKOCharacterBase::OnJumpStrengthChanged(float OldValue, float NewValue)
{
	GetCharacterMovement()->JumpZVelocity = NewValue;
}

