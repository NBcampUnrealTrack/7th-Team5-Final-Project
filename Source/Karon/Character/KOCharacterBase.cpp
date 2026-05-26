#include "KOCharacterBase.h"
#include "AbilitySystem/Attribute/KOHealthSet.h"
#include "AbilitySystem/Attribute/KOMovementSet.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Animation/KOAnimInstance.h"
#include "Component/KOCharacterMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Karon/AbilitySystem/KOAbilitySystemComponent.h"


AKOCharacterBase::AKOCharacterBase(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer.SetDefaultSubobjectClass<UKOCharacterMovementComponent>(
		ACharacter::CharacterMovementComponentName))
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
}

void AKOCharacterBase::BindMovementSet()
{
	MovementSet->OnMoveSpeedChanged.AddDynamic(this, &ThisClass::OnMoveSpeedChanged);
	OnMoveSpeedChanged(0.f, MovementSet->GetMoveSpeed());
	
	MovementSet->OnJumpStrengthChanged.AddDynamic(this, &ThisClass::OnJumpStrengthChanged);
	OnJumpStrengthChanged(0, MovementSet->GetJumpStrength());
}

void AKOCharacterBase::OnMoveSpeedChanged(float OldWalkSpeed, float NewWalkSpeed)
{
	GetCharacterMovement()->MaxWalkSpeed = NewWalkSpeed;
	
	UE_LOG(LogTemp, Log, TEXT("MoveSpeed Changed: %.2f -> %.2f"), OldWalkSpeed ,NewWalkSpeed);
}

void AKOCharacterBase::OnJumpStrengthChanged(float OldJumpStrength, float NewJumpStrength)
{
	GetCharacterMovement()->JumpZVelocity = NewJumpStrength;
}

