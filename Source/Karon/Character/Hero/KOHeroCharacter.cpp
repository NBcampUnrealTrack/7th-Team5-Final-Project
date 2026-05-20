#include "KOHeroCharacter.h"

#include "AbilitySystem/KOAbilitySystemComponent.h"
#include "AbilitySystem/Attribute/KOCombatSet.h"
#include "AbilitySystem/Attribute/KOStaminaSet.h"
#include "Game/KOPlayerState.h"


AKOHeroCharacter::AKOHeroCharacter(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	
	StaminaSet = CreateDefaultSubobject<UKOStaminaSet>(FName("StaminaSet"));
	CombatSet = CreateDefaultSubobject<UKOCombatSet>(FName("CombatSet"));
}


void AKOHeroCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void AKOHeroCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	AKOPlayerState* PS = GetPlayerState<AKOPlayerState>();
	if (!PS) return;
	
	AbilitySystemComponent = Cast<UKOAbilitySystemComponent>(PS->GetAbilitySystemComponent());
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->GiveDefaultAbilities(); 
		AbilitySystemComponent->InitAbilityActorInfo(PS, this);
	}
}

void AKOHeroCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->ProcessAbilityInput(DeltaTime, false);
	}

}

void AKOHeroCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

