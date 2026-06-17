#include "KOCharacterBase.h"
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
	
	EquipmentComponent = CreateDefaultSubobject<UKOEquipmentComponent>(TEXT("EquivalentComponent"));
}

UAbilitySystemComponent* AKOCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AKOCharacterBase::BeginPlay()
{
	Super::BeginPlay();
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
}

